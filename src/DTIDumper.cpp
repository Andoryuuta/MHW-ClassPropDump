#include "DTIDumper.hpp"

#include <Windows.h>
#include <cstdint>
#include <vector>
#include <map>
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
#include "config.h"

std::string GetDateTime() {
	time_t t = std::time(nullptr);
	struct tm timeinfo;
	localtime_s(&timeinfo, &t);
	std::ostringstream oss;
	oss << std::put_time(&timeinfo, "%d-%m-%Y %H-%M-%S");
	return oss.str();
}

namespace DTIDumper {

	uint64_t GetVftableByInstanciation(Mt::MtDTI* dti);
	void PopulateClassRecordProperties(ClassRecord* cr, std::map<std::string, Mt::MtDTI*>* dtiMap);

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


		SigScan::SuspendThreads();

		// Sig scan for the DTI hash table.
		// 48 8D 0D F778F002 - lea rcx,[MonsterHunterWorld.exe+50930D0]
		uint64_t hashTableAOB = NULL;
		hashTableAOB = SigScan::Scan(image_base, "48 8D ?? ?? ?? ?? 02 48 8D 0C C1 48 8B 01 48 85 C0 74 06 48 8D 48 28 EB F2");
		if (hashTableAOB == 0) {
			// Monster Hunter Stories 2 initial PC release:
			hashTableAOB = SigScan::Scan(image_base, "48 8D ?? ?? ?? ?? ?? 48 8D 0C C1 48 8B 01 48 85 C0 74 18");
			if (hashTableAOB == 0) {
				spdlog::critical("Failed to get MtDTI hash table");
			}
		}

		// Get displacement and calculate final value (displacement values count from next instr pointer, so add the opcode size (7)).
		uint32_t disp = *(uint32_t*)(hashTableAOB + 3);
		this->mt_dti_hash_table = (Mt::MtDTIHashTable*)(hashTableAOB + disp + 7);
		spdlog::info("mt_dti_hash_table: {0:x}", (uint64_t)mt_dti_hash_table);

		class_records = GetClassRecords();
	}


	std::vector<ClassRecord> DTIDumper::GetClassRecords() {
		auto dtiMap = GetFlattenedDtiMap();
		auto dtis = GetSortedDtiVector();

		/*
		* Find all of the (potential) `GetDTI` vftable methods:
		*    lea     rax, off_XXXXXXXX
		*    retn
		*/
		auto potential_getdti_aob_matches = SigScan::ScanAll(this->image_base, "48 8D 05 ?? ?? ?? ?? C3");
		spdlog::info("Getter AOB matches: 0x{0:x}", potential_getdti_aob_matches.size());

		// Create a map of offsets used in the found getter functions, mapped to the getter function address.
		std::map<uint64_t, uint64_t> lea_offset_to_getter_map;
		for (size_t i = 0; i < potential_getdti_aob_matches.size(); i++)
		{
			uint32_t displ = *(uint32_t*)(potential_getdti_aob_matches[i] + 3);
			lea_offset_to_getter_map[displ + potential_getdti_aob_matches[i] + 7] = potential_getdti_aob_matches[i];
		}
		spdlog::info("Generated gotten address <-> DTI getter map. Size: {0:X}", lea_offset_to_getter_map.size());


		std::vector<Mt::MtDTI*> undetermined_dti;
		std::map<Mt::MtDTI*, uint64_t> dti_to_vftable;

		// Go over each DTI and see if we found a _single_ getter for it. 
		for (size_t i = 0; i < dtis.size(); i++)
		{
			if (lea_offset_to_getter_map.find((uint64_t)dtis[i]) != lea_offset_to_getter_map.end())
			{
				uint64_t get_dti_vf_address = lea_offset_to_getter_map[(uint64_t)dtis[i]];
				std::vector<uint64_t> found = SigScan::AlignedUint64ListScan(this->image_base, get_dti_vf_address, PAGE_READONLY);


				if (found.size() != 1) {
					//spdlog::info("Got {0} matches for vf {1}::GetDTI ptr: 0x{2:X}", found.size(), dtis[i]->class_name, lea_offset_to_getter_map[(uint64_t)dtis[i]]);
					undetermined_dti.push_back(dtis[i]);
				}
				else {

					// Offset backward in the vftable to get to the base.
					// The ::GetDTI is the 4th virtual function.
					dti_to_vftable[dtis[i]] = found[0] - (8 * 4);
					//spdlog::info("Got single match for vf ptr->{0}::GetDTI 0x{1:X}", dtis[i]->class_name, found[0]);
				}
			}
			else
			{
				spdlog::info("Missing {0}::GetDTI: 0x{1:X}", dtis[i]->class_name, lea_offset_to_getter_map[(uint64_t)dtis[i]]);
				undetermined_dti.push_back(dtis[i]);
			}

			if (i % 100 == 0) {

				spdlog::info("DTI->vftable mapping progress: {0}/{1}", i, dtis.size());
			}
		}
		spdlog::info("Found total dti <-> vftable mapping(s): {0}", dti_to_vftable.size());
		spdlog::info("Undetermined dti <-> vftable mapping(s): {0}", undetermined_dti.size());

		if (undetermined_dti.size() > 0) {
			spdlog::info("Attempting to resolve undetermined vftable through instanciation");
			for (size_t i = 0; i < undetermined_dti.size(); i++)
			{

				spdlog::info("Trying to resolve: {0}", undetermined_dti[i]->class_name);
				auto found = GetVftableByInstanciation(undetermined_dti[i]);
				if (found != 0) {
					dti_to_vftable[undetermined_dti[i]] = found;
				}
			}

			spdlog::info("New total dti <-> vftable mapping(s): {0}", dti_to_vftable.size());
		}

		// Create the class records with populated properties
		//class_records.push_back(ClassRecord{ dti, obj, obj_vftable, properties });

		std::ofstream debug_file;
		debug_file.open("dti_dumper_debug_log.txt", std::ios::out | std::ios::trunc);


		std::vector<ClassRecord> class_records;
		for (auto const& [dti, vftable_addr] : dti_to_vftable)
		{
			debug_file << fmt::format("Getting records for DTI: {0}. Vftable: {1:X} \n", dti->class_name, vftable_addr) << std::flush;

			ClassRecord record;
			record.dti = dti;
			record.obj_instance = nullptr;
			record.class_vftable = (void*)vftable_addr;

			// Skip over excluded DTI classes.
			if (this->excluded_classes.find(dti->class_name) == this->excluded_classes.end()) {
				PopulateClassRecordProperties(&record, &dtiMap);
			} else {
				spdlog::warn("Skipping property population of excluded class: {0}", dti->class_name);
			}

			class_records.push_back(record);
		}
		return class_records;
	}

	/*
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

				std::sort(properties.begin(), properties.end(), [&](const auto& lhs, const auto& rhs) {
					return lhs->GetFieldOffsetFrom((uint64_t)obj) < rhs->GetFieldOffsetFrom((uint64_t)obj);
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
	*/

	uint64_t GetVftableByInstanciation(Mt::MtDTI* dti) {
		// Create an instance of the object we are dumping the properties of.
		Mt::MtObject* mtObj = nullptr;
		__try {
			mtObj = reinterpret_cast<Mt::MtObject*>(dti->NewInstance());
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			spdlog::error("Got error instanciating {0}", dti->class_name);
			return 0;
		}

		return *(uint64_t*)mtObj;
	}

	void PopulateClassRecordProperties(ClassRecord* cr, std::map<std::string, Mt::MtDTI*>* dtiMap) {
		__try
		{
			// Create a MtPropertyList to populate with the MtObject::PopulatePropertyList function.
			Mt::MtPropertyList* propListInst = nullptr;
			__try {
				auto mtPropListDTI = (*dtiMap)["MtPropertyList"];
				propListInst = reinterpret_cast<Mt::MtPropertyList*>(mtPropListDTI->NewInstance());
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				spdlog::error("Error instanciating MtPropertyList!");
				return;
			}

			spdlog::info("Prop list: {0:X}", (uint64_t)propListInst);

			// Populate our property list with the overridden virtual function on the MtObject.
			__try
			{
				typedef bool(__thiscall* PopulatePropertyList_t)(Mt::MtObject*, Mt::MtPropertyList*);
				auto address = (uint64_t)cr->class_vftable + (3 * 8);
				PopulatePropertyList_t fp = (PopulatePropertyList_t)(*(uint64_t*)address);
				spdlog::info("Calling {0}::GetDTI vf ptr: {1:X}", cr->dti->class_name, address);

				fp(nullptr, propListInst);
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				spdlog::error("Got error populating property list for {0}", cr->dti->class_name);
				return;
			}

			if (propListInst->first_prop != nullptr) {
				for (Mt::MtProperty* prop = propListInst->first_prop; prop != nullptr; prop = prop->next) {
					cr->properties.push_back(prop);
				}
			}

			std::sort(cr->properties.begin(), cr->properties.end(), [&](const auto& lhs, const auto& rhs) {
				return lhs->GetFieldOffsetFrom((uint64_t)0) < rhs->GetFieldOffsetFrom((uint64_t)0);
				});
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			spdlog::error("Got SEH exception while printing properties for {0}", cr->dti->class_name);
		}
	}

	void DTIDumper::DumpRawInfo(std::string filename) {
		
	}

	void DTIDumper::DumpToFile(std::string filename) {
		//auto class_records = GetClassRecords();

		spdlog::info("Starting file dump.");
		Sleep(1000);

		std::ofstream file;
		file.open(filename, std::ios::out | std::ios::trunc);

		file << fmt::format("// Ando's MT DTI dump log [{}]\n", GetDateTime());
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

				file << fmt::format("class {} /*{}*/ {{\n", rec.dti->class_name, inheritanceString) << std::flush;

				// Different formatting based on the property type.
				for (const auto& prop : rec.properties) {
#if defined(MT_GAME_MHW)
					if (prop->prop_comment != nullptr) {
						file << fmt::format("\t// Comment: {0}\n", prop->prop_comment);
					}
#endif

					std::string typeAndName = fmt::format("{0} '{1}'", prop->GetTypeName(), prop->prop_name);
					//int64_t cappedOffset = prop->GetFieldOffset();
					int64_t cappedOffset = prop->GetFieldOffsetFrom((uint64_t)rec.obj_instance);
					if (cappedOffset < 0 || cappedOffset > rec.dti->ClassSize()) {
						cappedOffset = INT64_MAX;
					}

					if (prop->IsArrayType()) {
						if (prop->IsGetterSetter()) {
							// Dynamic array
							std::string varString = fmt::format("{0}[*]", typeAndName, prop->data.var.count);
							file << fmt::format("\t{0:<50}; // Offset:0x{1:X}, DynamicArray, Getter:0x{2:X}, Setter:0x{3:X}, GetCount:0x{4:X}, Reallocate:0x{5:X} CRC32:0x{6:X}, Flags:0x{7:X}\n", varString, cappedOffset, prop->data.gs.fp_get, prop->data.gs.fp_set, prop->data.gs.fp_get_count, prop->data.gs.fp_dynamic_allocation, prop->GetCRC32(), prop->GetFullFlags()) << std::flush;
						}
						else
						{
							// Static/fixed-length array
							std::string varString = fmt::format("{0}[{1}]", typeAndName, prop->data.var.count);
							file << fmt::format("\t{0:<50}; // Offset:0x{1:X}, Array, CRC32:0x{2:X}, Flags:0x{3:X}\n", varString, cappedOffset, prop->GetCRC32(), prop->GetFullFlags()) << std::flush;
						}
					}
					else if (prop->IsGetterSetter()) {
						file << fmt::format("\t{0:<50}; // Offset:0x{1:X}, PSEUDO-PROP, Getter:0x{2:X}, Setter:0x{3:X}, CRC32:0x{4:X}, Flags:0x{5:X}\n", typeAndName, cappedOffset, (uint64_t)prop->data.gs.fp_get, (uint64_t)prop->data.gs.fp_set, prop->GetCRC32(), prop->GetFullFlags()) << std::flush;
					}
					else {
						file << fmt::format("\t{0:<50}; // Offset:0x{1:X}, Var, CRC32:0x{2:X}, Flags:0x{3:X}\n", typeAndName, cappedOffset, prop->GetCRC32(), prop->GetFullFlags()) << std::flush;
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

	void DTIDumper::DumpPythonArrayFile(std::string filename) {
		spdlog::info("Starting python tuple list file dump.");
		Sleep(1000);

		std::ofstream file;
		file.open(filename, std::ios::out | std::ios::trunc);

		file << fmt::format("# Ando's MHW ClassPropDump log [{}]\n", GetDateTime());
		file << fmt::format("# Image Base: {:#X}\n\n", this->image_base);
		file << fmt::format("# class name, vftable address, dti address\n.");

		file << fmt::format("dti_data = [");

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
				
				file << fmt::format("('{0}', 0x{1:X}, 0x{2:X}),\n", rec.dti->class_name, (uint64_t)rec.class_vftable, (uint64_t)rec.dti);
				
			}
			catch (const std::exception& e)
			{
				spdlog::error("Error: {}", e.what());
				Sleep(5000);
			}
		}
		file << fmt::format("]");
		file << fmt::format("# END OF FILE");

		spdlog::info("Done dumping classes!");
	}

	void DTIDumper::DumpResourceInformation(std::string filename) {
		std::ofstream file;
		file.open(filename, std::ios::out | std::ios::trunc);

		file << fmt::format("// cResource subclass information. [{}]\n", GetDateTime());
		file << fmt::format("// Image Base: {:#X}\n\n", this->image_base);

		spdlog::info("Dumping resource information classes!");
		// Output as text.
		for (const auto& rec : class_records) {
			__try {
				auto isResourceClass = rec.dti->IsSubclassOf("cResource");

				//spdlog::info("is resource class? {0} - {1}", rec.dti->class_name, isResourceClass);

				if (isResourceClass) {
					if (rec.class_vftable != nullptr) {
						uint64_t addr = *(uint64_t*)(((uint64_t)rec.class_vftable) + (8 * 6));
						typedef char* (*GetResourceExtension_t)();
						GetResourceExtension_t GetExt = (GetResourceExtension_t)(addr);
						char* ext = GetExt();

						file << fmt::format("Class [{0}], file extension: {1}\n", rec.dti->class_name, ext);
						file.flush();
					}
				}
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				spdlog::error("Error dumping resource information!");
				Sleep(5000);
				continue;
			}
		}
		spdlog::info("Done dumping resource information classes!");
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