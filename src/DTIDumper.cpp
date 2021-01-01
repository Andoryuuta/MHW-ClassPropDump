#include "DTIDumper.hpp"

#include <Windows.h>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <ctime>
#include <sstream>
#include "spdlog/spdlog.h"
#include <fmt/core.h>
#include "SigScan.hpp"
#include "Mt.hpp"
#include "crc.h"

std::string GetDateTime() {
	time_t t = std::time(nullptr);
	struct tm timeinfo;
	localtime_s(&timeinfo, &t);
	std::ostringstream oss;
	oss << std::put_time(&timeinfo, "%d-%m-%Y %H-%M-%S");
	return oss.str();
}

namespace DTIDumper {
	DTIDumper::DTIDumper()
	{
		image_base = 0;
		mt_dti_hash_table = nullptr;
	}

	DTIDumper::~DTIDumper()
	{
	}

	void DTIDumper::ParseDTI() {

		// Get the image base.
		this->image_base = (uint64_t)GetModuleHandle(NULL);
		spdlog::info("Image Base: {0:x}", image_base);

		// Sig scan for the DTI hash table.
		// 48 8D 0D F778F002 - lea rcx,[MonsterHunterWorld.exe+50930D0]
		uint64_t hashTableAOB = SigScan::Scan(image_base, "48 8D ?? ?? ?? ?? 02 48 8D 0C C1 48 8B 01 48 85 C0 74 06 48 8D 48 28 EB F2");
		if (hashTableAOB == 0) {
			spdlog::critical("Failed to get MtDTI hash table");
		}

		// Get displacement and calculate final value (displacement values count from next instr pointer, so add the opcode size (7)).
		uint32_t disp = *(uint32_t*)(hashTableAOB + 3);
		this->mt_dti_hash_table = (Mt::MtDTIHashTable*)(hashTableAOB + disp + 7);
		spdlog::info("mt_dti_hash_table: {0:x}", (uint64_t)mt_dti_hash_table);
	}

	std::vector<ClassRecord> DTIDumper::GetClassRecords() {
		// Get the DTI map.
		auto dtiMap = GetFlattenedDtiMap();
		auto dtiVecSorted = GetSortedDtiVector();

		std::vector<ClassRecord> class_records;

		for (auto const& dti : dtiVecSorted) {
			// Skip over excluded DTI classes.
			if (this->excluded_classes.find(dti->class_name) != this->excluded_classes.end()) {

				spdlog::warn("Skipping excluded class: {0}", dti->class_name);
				continue;
			}


			// Get a vector of properties sorted by offset.
			std::vector<Mt::MtProperty*> properties;
			void* obj = nullptr;
			void* obj_vftable = nullptr;
			__try
			{

				// Create a MtPropertyList to populate with the MtObject::PopulatePropertyList function.
				Mt::MtPropertyList* propListInst = nullptr;
				__try {
					auto mtPropListDTI = dtiMap["MtPropertyList"];
					propListInst = reinterpret_cast<Mt::MtPropertyList*>(mtPropListDTI->CtorInstance(malloc(mtPropListDTI->ClassSize())));
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
					spdlog::error("Error instanciating MtPropertyList!");
					Sleep(15000);
					continue;
				}

				// Create an instance of the object we are dumping the properties of.
				Mt::MtObject* mtObj = nullptr;
				__try {
					mtObj = reinterpret_cast<Mt::MtObject*>(dti->CtorInstance(malloc(dti->ClassSize())));
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
					spdlog::error("Got error instanciating {0}", dti->class_name);
					continue;
				}


				if (mtObj == nullptr) {
					spdlog::warn("{0}::DTI->NewInstance() == nullptr", dti->class_name);
					continue;
				}

				obj = (void*)mtObj;
				obj_vftable = *(void**)mtObj;

				// Populate our property list with the overridden virtual function on the MtObject.
				__try
				{

					mtObj->PopulatePropertyList(propListInst);
				}
				__except (EXCEPTION_EXECUTE_HANDLER)
				{
					spdlog::error("Got error populating property list for {0}", dti->class_name);
					continue;
				}


				if (propListInst->first_prop != nullptr) {
					for (Mt::MtProperty* prop = propListInst->first_prop; prop != nullptr; prop = prop->next) {
						properties.push_back(prop);
					}
				}

				std::sort(properties.begin(), properties.end(), [](const auto& lhs, const auto& rhs) {
					return lhs->GetFieldOffset() < rhs->GetFieldOffset();
				});



			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				spdlog::error("Got SEH exception while printing properties for {0}", dti->class_name);
			}

			class_records.push_back(ClassRecord{ dti, obj, obj_vftable, properties });
		}

		return class_records;
	}

	void DTIDumper::DumpToFile(std::string filename) {
		auto class_records = GetClassRecords();

		spdlog::info("Starting file dump.");
		Sleep(5000);

		std::ofstream file;
		file.open(filename, std::ios::out | std::ios::trunc);

		file << fmt::format("// Ando's MHW ClassPropDump log [{}]\n", GetDateTime());
		file << fmt::format("// Image Base: {:#X}\n\n", this->image_base);

		// Output as text.
		for (const auto& rec : class_records) {
			// Generate the C++ styled inheritance chain for comment.
			std::stringstream inheritance;
			Mt::MtDTI* cur = rec.dti;
			if (cur->class_name != nullptr && std::strlen(cur->class_name) != 0) {
				inheritance << ": ";
			}
			cur = cur->parent;
			while (cur != nullptr && cur->parent != cur) {
				if (cur->class_name != nullptr && std::strlen(cur->class_name) != 0) {
					inheritance << cur->class_name << ", ";
				}
				cur = cur->parent;
			}
			std::string inheritanceString = inheritance.str();
			inheritanceString.resize(inheritanceString.size() - 2);

			// Output the class + properties to the file.
			try
			{
				// Output calculated CRC if it differs from the value in memory (MtDTI constructor allows CRC overrides, though these appear to be unused).
				uint32_t classNameJAMCRC = (get_cstr_crc(rec.dti->class_name) & 0x7FFFFFFF);
				if (rec.dti->crc_hash == classNameJAMCRC) {
					file << fmt::format("// {0} vftable:0x{1:X}, Size:0x{2:X}, CRC32:0x{3:X}\n", rec.dti->class_name, (uint64_t)rec.class_vftable, rec.dti->ClassSize(), rec.dti->crc_hash);
				}
				else {
					file << fmt::format("// {0} vftable:0x{1:X}, Size:0x{2:X}, CRC32:0x{3:X}, CalcCRC:0x{4:X}\n", rec.dti->class_name, (uint64_t)rec.class_vftable, rec.dti->ClassSize(), rec.dti->crc_hash, classNameJAMCRC);
				}

				file << fmt::format("class {} /*{}*/ {{\n", rec.dti->class_name, inheritanceString);

				// Different formatting based on the property type.
				for (const auto& prop : rec.properties) {
					std::string typeAndName = fmt::format("{0} '{1}'", prop->GetTypeName(), prop->prop_name);
					if (prop->IsArrayType()) {
						if (prop->IsGetterSetter()) {
							// Dynamic array
							std::string varString = fmt::format("{0}[*]", typeAndName, prop->data.var.count);
							file << fmt::format("\t{0:<50}; // Offset:0x{1:X}, DynamicArray, Getter:0x{2:X}, Setter:0x{3:X}, GetCount:0x{4:X}, Reallocate:0x{5:X} CRC32:0x{5:X}, Flags:0x{6:X}\n", varString, prop->GetFieldOffset(), prop->data.gs.fp_get, prop->data.gs.fp_set, prop->data.gs.fp_get_count, prop->data.gs.fp_dynamic_allocation, prop->GetCRC32(), prop->GetFullFlags());
						}
						else
						{
							// Static/fixed-length array
							std::string varString = fmt::format("{0}[{1}]", typeAndName, prop->data.var.count);
							file << fmt::format("\t{0:<50}; // Offset:0x{1:X}, Array, CRC32:0x{2:X}, Flags:0x{3:X}\n", varString, prop->GetFieldOffset(), prop->GetCRC32(), prop->GetFullFlags());
						}
					}
					else if (prop->IsGetterSetter()) {
						file << fmt::format("\t{0:<50}; // Offset:0x{1:X}, PSEUDO-PROP, Getter:0x{2:X}, Setter:0x{3:X}, CRC32:0x{4:X}, Flags:0x{5:X}\n", typeAndName, prop->GetFieldOffset(), (uint64_t)prop->data.gs.fp_get, (uint64_t)prop->data.gs.fp_set, prop->GetCRC32(), prop->GetFullFlags());
					}
					else {
						file << fmt::format("\t{0:<50}; // Offset:0x{1:X}, Var, CRC32:0x{2:X}, Flags:0x{3:X}\n", typeAndName, prop->GetFieldOffset(), prop->GetCRC32(), prop->GetFullFlags());
					}
				}

				file << fmt::format("}};\n\n", rec.dti->class_name);
			}
			catch (const std::exception& e)
			{
				spdlog::error("Error: {}", e.what());
				Sleep(5000);
			}
		}
		file << fmt::format("// END OF FILE");

		spdlog::info("Done dumping classes!");
	}


	std::map<std::string, Mt::MtDTI*> DTIDumper::GetFlattenedDtiMap() {
		// Iterate over the hash buckets to build a flat lookup map
		// (lowest byte of the class name CRC32 is used as a bucket index normally).
		std::map<std::string, Mt::MtDTI*> dtis;
		for (int bucketIndex = 0; bucketIndex < 256; bucketIndex++) {
			Mt::MtDTI* dti = mt_dti_hash_table->buckets[bucketIndex];
			while (true) {
				//spdlog::info("DTI@{0:X} = {1}, CRC32:{2:x}", (uint64_t)dti, dti->class_name, dti->crc_hash);
				dtis[dti->class_name] = dti;

				if (dti->next == nullptr)
					break;
				dti = dti->next;
			}
		}

		return dtis;
	}

	std::vector<Mt::MtDTI*> DTIDumper::GetSortedDtiVector() {
		// Iterate over the hash buckets to build a vector.
		std::vector<Mt::MtDTI*> dtis;
		for (int bucketIndex = 0; bucketIndex < 256; bucketIndex++) {
			Mt::MtDTI* dti = mt_dti_hash_table->buckets[bucketIndex];
			while (true) {
				dtis.push_back(dti);

				if (dti->next == nullptr)
					break;
				dti = dti->next;
			}
		}

		std::sort(dtis.begin(), dtis.end(), [](const auto& lhs, const auto& rhs) {
			if (lhs->class_name == nullptr || rhs->class_name == nullptr) {
				return false;
			}
			return std::strcmp(lhs->class_name, rhs->class_name) < 0;
		});

		return dtis;
	}
}