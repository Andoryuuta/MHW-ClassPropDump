#include <format>
#include <fstream>
#include <iostream>
#include <set>
#include <windows.h>

#include "dumper.h"
#include "mt/mt.h"
#include "sig_scan.h"

#include "spdlog/spdlog.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace Dumper
{
void to_json(json& j, const ProcessedClassRecord& c)
{
    // clang-format off
    std::vector<uintptr_t> class_vftable_vas;
    for (auto&& cvft : c.class_vftables)
    {
        class_vftable_vas.push_back(cvft);
    }
    
    json prop_list;
    for (auto &&pvft : c.properties)
    {
        json prop;
        for (auto &&pp : pvft.properties)
        {
            json p = json{
                //{"name", pp.name},
                 {"name_bytes", pp.name_bytes},
                {"type", pp.type},
                {"attr", pp.attr},
                {"owner", pp.owner},
                {"get", pp.get},
                {"get_count", pp.get_count},
                {"set", pp.set},
                {"set_count", pp.set_count},
                {"index", pp.index}
            };
            prop.push_back(p);
        }

        json jpvft = json{
            {"vftable_va", pvft.vftable_va},
            {"properties", prop}
        };
        prop_list.push_back(jpvft);   
    }
    

    // clang-format off
    j = json{
        {"image_base", c.image_base},
        {"dti_va", c.dti_va},
        {"dti_vftable_va", c.dti_vftable_va},
        {"id", c.id},
        {"name", c.name},
        {"parent_id", c.parent_id},
        {"raw_flags_and_size", c.raw_flags_and_size},
        {"size", c.size},
        {"allocator_index", c.allocator_index},
        {"is_abstract", c.is_abstract},
        {"is_hidden", c.is_hidden},
        {"properties_processed", c.properties_processed}, 
        {"class_vftable_vas", class_vftable_vas},
        {"properties", prop_list}
    };
    // clang-format off
}

void from_json(const json& j, ProcessedClassRecord& c)
{
    // clang-format off
    j.at("dti_va").get_to(c.dti_va);
    j.at("dti_vftable_va").get_to(c.dti_vftable_va);
    j.at("id").get_to(c.id);
    j.at("name").get_to(c.name);
    j.at("parent_id").get_to(c.parent_id);
    j.at("raw_flags_and_size").get_to(c.raw_flags_and_size);
    j.at("size").get_to(c.size);
    j.at("allocator_index").get_to(c.allocator_index);
    j.at("is_abstract").get_to(c.is_abstract);
    j.at("is_hidden").get_to(c.is_hidden);
    j.at("properties_processed").get_to(c.properties_processed);
    // clang-format off
}

Dumper::Dumper(uintptr_t image_base) : m_image_base(image_base), m_dti_hash_table(nullptr), m_sorted_dti_vec(), m_class_records(8192) {}
Dumper::~Dumper() {
    m_sorted_dti_vec.clear();
    m_class_records.clear();
}

void Dumper::Initialize()
{
    m_dti_hash_table = GetDTIHashTable(m_image_base);
    if (m_dti_hash_table == nullptr)
    {
        throw std::runtime_error("Failed to get MT_DTI_HASH_TABLE");
    }

    m_sorted_dti_vec = this->GetSortedDTIVector();
}

void Dumper::BuildClassRecords()
{
    // Find all of the (potential) `GetDTI` vftable methods:
    /*
    *    lea     rax, off_XXXXXXXX
    *    retn
    */
    // TODO(Andorytuuta): Make 32-bit variant of this logic.
    std::vector<uintptr_t> potential_getdti_matches = SigScan::ScanCurrentExecutable("48 8D 05 ?? ?? ?? ?? C3", /*alignment=*/8);
    spdlog::info("Potential ::GetDTI AOB matches: 0x{0:x}", potential_getdti_matches.size());


    // Build a set in order to have a fast check if an address is a DTI object instance.
    std::set<Mt::MtDTI*> dti_set;
    for (auto &&dti : m_sorted_dti_vec)
    {
        dti_set.insert(dti);
    }

    // Build a map of all vftables that are associated with a single DTI.
    // Because of inheritance outside of the DTI sysmtem, there may be multiple
    // ::GetDTI functions for the same DTI object.
    // 
    // For each match we:
    //     1. Get the address loaded by the `lea rax, XXXXXXXX` instruction we scanned for
    //     2. Check if it's a DTI object that we know about.
    //     3. Scan for this instructions address (since it's the first instruction in the function)
    //        in order to locate the vftable that it's defined in.
    std::map<Mt::MtDTI*, std::vector<uintptr_t>> dti_class_vftables;
    for (size_t i = 0; i < potential_getdti_matches.size(); i++)
    {
        if (i % 100 == 0) 
        {
            spdlog::info("Processing ::GetDTI matches [{}/{}]", i, potential_getdti_matches.size());
        }
        uintptr_t match_address = potential_getdti_matches[i];
        uint32_t displ = *(uint32_t*)(match_address + 3);
        uint32_t x64_lea_rax_opcode_size = 7;
        uintptr_t constant_address = displ + match_address + x64_lea_rax_opcode_size;

        Mt::MtDTI* object = reinterpret_cast<Mt::MtDTI*>(constant_address);
        if(dti_set.contains(object))
        {
            // We know the getter is for a valid DTI object.
            // Find the vftable(s) that this getter is listed in.

            // spdlog::info("Searching vftable for getter:{:X}, DTI:{}", match_address, m_sorted_dti_vec[i]->name());

            std::vector<uintptr_t> vftable_matches = SigScan::ScanCurrentExecutableUintptrAligned(match_address, PAGE_READONLY);
            for (auto &&vftable_match : vftable_matches)
            {
                // Since we have the ::GetMethod address in the vftable, 
                // we need to walk back to the start of the vftable.
                // We do this by stepping back 4 pointers (see MtObject definition).
                dti_class_vftables[object].push_back(vftable_match - (sizeof(uintptr_t)*4));
            }
        }


        // // Validate that the constant is a known DTI object.
        // // TODO(Andoryuuta): O(N*M) complexity - build a temporary map/set for faster DTI lookup.
        // for (size_t j = 0; j < m_sorted_dti_vec.size(); j++)
        // {
        //     if (constant_address == reinterpret_cast<uintptr_t>(m_sorted_dti_vec[j]))
        //     {
        //         // We know the getter is for a valid DTI object.
        //         // Find the vftable(s) that this getter is listed in.

        //         // spdlog::info("Searching vftable for getter:{:X}, DTI:{}", match_address, m_sorted_dti_vec[i]->name());

        //         std::vector<uintptr_t> vftable_matches = SigScan::ScanCurrentExecutableUintptrAligned(match_address);
        //         for (auto &&vftable_match : vftable_matches)
        //         {
        //             // Since we have the ::GetMethod address in the vftable, 
        //             // we need to walk back to the start of the vftable.
        //             // We do this by stepping back 4 pointers (see MtObject definition).
        //             dti_class_vftables[m_sorted_dti_vec[j]].push_back(vftable_match - (sizeof(uintptr_t)*4));
        //         }
        //         break;
        //     }
        // }
    }

    // Build the class records from the DTI map.
    m_class_records.clear();
    for (auto&& dti : m_sorted_dti_vec)
    {
        std::vector<uintptr_t> class_vftable_vas;
        if (dti_class_vftables.contains(dti)) {
            class_vftable_vas = dti_class_vftables[dti];
        }

        ProcessedClassRecord record = {
            .image_base = this->m_image_base,
            .dti = reinterpret_cast<uintptr_t>(dti),
            .dti_va = dti->dti_va(),
            .dti_vftable_va = dti->dti_vftable_va(),
            .id = dti->id(),
            .name = std::string(dti->name()),
            .parent_id = dti->parent_id(),
            .raw_flags_and_size = dti->raw_flags_and_size(),
            .size = dti->size(),
            .allocator_index = dti->allocator_index(),
            .is_abstract = dti->is_abstract(),
            .is_hidden = dti->is_hidden(),
            .class_vftables = class_vftable_vas,
        };
        m_class_records.push_back(record);
    }
}

// Dump the core DTI map (no properties). 
void Dumper::DumpDTIMap(std::string_view filename)
{
    spdlog::info("Writing base DTI map to: {}", filename);

    json j = m_class_records;

    std::ofstream file;
    file.open(filename, std::ios::out | std::ios::trunc);
    file << j.dump();
    file.flush();
}

// Dumps the list of class, hash, and file extension for
// all cResource dependants.
//
// This must only be called after the m_class_records vector
// has been built.
void Dumper::DumpResourceInformation(std::string_view filename)
{
    spdlog::info("Writing cResource info to: {}", filename);

    std::ofstream file;
    file.open(filename, std::ios::out | std::ios::trunc);

    for (auto &&record: this->m_class_records)
    {
        Mt::MtDTI* dti = reinterpret_cast<Mt::MtDTI*>(record.dti_va);
        file << fmt::format("Class:{}\n", dti->name());
        file.flush();

        if(dti->IsDependantOf("cResource"))
        {
            if(record.class_vftables.size() >= 1)
            {
                // We build a fake object here (just a pointer to the vftable)
                // in order to call the getExt function. We don't need to create
                // a real object since it just returns a static const char*. 
                uintptr_t tmp = record.class_vftables[0];
                Mt::cResource* rdti = reinterpret_cast<Mt::cResource*>(&tmp);
                file << fmt::format("Class:{}, ID:{:X}, Extension:{}\n", dti->name(), dti->id(), rdti->getExt());
            }
        }
    }

    file.flush();
    spdlog::info("Done writing cResource info");
}

// Get a flat vector of DTI pointers, sorted alphabetically by name.
std::vector<Mt::MtDTI*> Dumper::GetSortedDTIVector()
{
    // Iterate over the hash buckets to build a vector.
    std::vector<Mt::MtDTI*> entries;
    for (int i = 0; i < 256; i++)
    {
        Mt::MtDTI* dti = m_dti_hash_table->buckets[i];
        while (true)
        {
            entries.push_back(dti);

            if (dti->link() == nullptr)
                break;
            dti = dti->link();
        }
    }

    // Sort by name string.
    std::sort(entries.begin(), entries.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs->name() == nullptr || rhs->name() == nullptr)
        {
            return false;
        }
        return std::strcmp(lhs->name(), rhs->name()) < 0;
    });

    return entries;
}

// Get the DTI object for a given class name (returns nullptr if not found).
Mt::MtDTI* Dumper::GetDTIByName(std::string_view name)
{
    for (auto &&dti : m_sorted_dti_vec)
    {
        if(dti->name() == name)
        {
            return dti;
        }
    }
    
    return nullptr;
}

// A helper struct for modeling MSVC's memory layout for classes with virtual functions.
struct MSVCVirtualClassInstance 
{
    uintptr_t vftable;
};


// TODO(Andoryuuta): Hand this possibly-failing task onto a seperate thread that we can suspend/terminate if it goes crazy.
//bool try_dti_create_property(Mt::MtObject* object, Mt::MtPropertyList* property_list)
//{
//    __try
//    {
//        object->CreateProperty(property_list);
//        return true;
//    }
//    __except (EXCEPTION_EXECUTE_HANDLER)
//    {
//        return false;
//    }
//}


bool try_dti_create_property(Mt::MtObject* object, Mt::MtPropertyList* property_list)
{
    __try
    {
        object->CreateProperty(property_list);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

struct try_dti_create_property_threaded_entry_struct {
    Mt::MtObject* object;
    Mt::MtPropertyList* property_list;
    bool result;
    bool finished_ok;
};

DWORD WINAPI try_dti_create_property_threaded_entry(LPVOID lpvParam)
{
    auto data = (try_dti_create_property_threaded_entry_struct*)lpvParam;
    data->result = try_dti_create_property(data->object, data->property_list);
    data->finished_ok = true;
    return 0;
}

bool try_dti_create_property_threaded(Mt::MtObject* object, Mt::MtPropertyList* property_list)
{
    struct try_dti_create_property_threaded_entry_struct data = {
        .object = object,
        .property_list = property_list,
        .finished_ok = false,
    };
    auto handle = CreateThread(NULL, 0, try_dti_create_property_threaded_entry, (LPVOID)&data, 0, NULL);
    auto wait_result = WaitForSingleObject(handle, 1000);
    if (wait_result == WAIT_TIMEOUT)
    {
        TerminateThread(handle, 1);
        spdlog::info("try_dti_create_property thread timed out/failed - terminating thread.");
        return false;
    }

    if (data.finished_ok)
    {
        spdlog::info("try_dti_create_property failed.");
        return false;
    }

    return data.result;
}





Mt::MtObject* try_dti_create_new_instance(Mt::MtDTI* dti)
{
    __try
    {
        Mt::MtObject* obj = reinterpret_cast<Mt::MtObject*>(dti->NewInstance());
        return obj;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return nullptr;
    }
}

struct try_dti_create_new_instance_threaded_entry_struct {
    Mt::MtDTI* dti;
    Mt::MtObject* result;
    bool finished_ok;
};

DWORD WINAPI try_dti_create_new_instance_threaded_entry(LPVOID lpvParam)
{
    auto data = (try_dti_create_new_instance_threaded_entry_struct*)lpvParam;
    data->result = try_dti_create_new_instance(data->dti);
    data->finished_ok = true;
    return 0;
}

Mt::MtObject* try_dti_create_new_instance_threaded_entry_struct(Mt::MtDTI* dti)
{
    struct try_dti_create_new_instance_threaded_entry_struct data = {
        .dti = dti,
        .result = nullptr,
        .finished_ok = false,
    };
    auto handle = CreateThread(NULL, 0, try_dti_create_new_instance_threaded_entry, (LPVOID)&data, 0, NULL);
    auto wait_result = WaitForSingleObject(handle, 1000);
    if (wait_result == WAIT_TIMEOUT)
    {
        TerminateThread(handle, 1);
        spdlog::info("try_dti_create_new_instance thread timed out/failed - terminating thread.");
        return nullptr;
    }

    if (data.finished_ok)
    {
        spdlog::info("try_dti_create_new_instance failed.");
        return nullptr;
    }

    return data.result;
}




















// Process all of the DTI properties of the current class records.
void Dumper::ProcessProperties()
{
    std::ofstream file;
    file.open("property_process.log", std::ios::out | std::ios::trunc);

    for (size_t i = 0; i < m_class_records.size(); i++)
    {
        ProcessedClassRecord class_record = m_class_records[i];
        Mt::MtDTI* dti = reinterpret_cast<Mt::MtDTI*>(class_record.dti);
        spdlog::info("Processing properties for {}, vftable count: {}", dti->name(), class_record.class_vftables.size());
        file << std::format("Processing properties for {}, vftable count: {}\n", dti->name(), class_record.class_vftables.size());

        for (size_t vftable_idx = 0; vftable_idx < class_record.class_vftables.size(); vftable_idx++)
        {
            spdlog::info("Processing properties for {}, vftable idx: {}", dti->name(), vftable_idx);
            file << std::format("Processing properties for {}, vftable idx: {}\n", dti->name(), vftable_idx);

            // Create a fake object initalized to zero to hold our fake object.
            // Because of inheiritance outside the DTI system, we don't know that the
            // size is 100% accurate, nor that the `createProperty` method wont try
            // to access memory outside these bytes, so we add some extra room "just-in-case"
            // (predictable crashes with nullptr are better than it mysteriously "working" sometimes)
            std::vector<uint8_t> fake_object_memory(dti->size(), 0);
            {
                auto fake_object = reinterpret_cast<MSVCVirtualClassInstance*>(fake_object_memory.data());
                fake_object->vftable = class_record.class_vftables[vftable_idx];
            }
            auto fake_object = reinterpret_cast<Mt::MtObject*>(fake_object_memory.data());
            // spdlog::info("Created fake object: 0x{:X}", reinterpret_cast<uintptr_t>(fake_object));
            file << std::format("Created fake object: 0x{:X}\n", reinterpret_cast<uintptr_t>(fake_object));
        
            // Create a MtPropertyList
            auto mt_property_list_dti = this->GetDTIByName("MtPropertyList");
            if (mt_property_list_dti == nullptr)
            {
                throw std::runtime_error("Failed to get MtPropertyList DTI!");
            }

            Mt::MtPropertyList* property_list = reinterpret_cast<Mt::MtPropertyList*>(mt_property_list_dti->NewInstance());
            // spdlog::info("Created MtPropertyList: 0x{:X}", reinterpret_cast<uintptr_t>(property_list));
            file << std::format("Created MtPropertyList: 0x{:X}\n", reinterpret_cast<uintptr_t>(property_list));
            file.flush();

            // Populate the properties into into.
            bool ok = try_dti_create_property_threaded(fake_object, property_list);
            //if (!ok)
            //{
            //    spdlog::info("Hard exception while processing fake object, making real object!");
            //    file << std::format("Hard exception while processing fake object, making real object!\n");

            //    if(dti->is_abstract())
            //    {
            //        spdlog::critical("DTI is abstract - can't attempt to create a real object. Skipping.");
            //        file << std::format("DTI is abstract - can't attempt to create a real object. Skipping.\n");
            //        continue;
            //    }

            //    // Create a new property list & real object.
            //    property_list = reinterpret_cast<Mt::MtPropertyList*>(mt_property_list_dti->NewInstance());
            //    Mt::MtObject* real_object = try_dti_create_new_instance_threaded_entry_struct(dti);
            //    if(real_object == nullptr)
            //    { 
            //        spdlog::critical("Failed to create new instance of real object!");
            //        file << std::format("Failed to create new instance of real object!\n");
            //        continue;
            //    }

            //    // Update real object vftable ptr in case of inheritance outside the DTI system.
            //    {
            //        auto tmp = reinterpret_cast<MSVCVirtualClassInstance*>(real_object);
            //        tmp->vftable = class_record.class_vftables[vftable_idx];
            //    }

            //    bool ok = try_dti_create_property_threaded(real_object, property_list);
            //    //if (!ok)
            //    //{
            //    //    spdlog::critical("Hard exception while processing real object!");
            //    //    file << std::format("Hard exception while processing real object!\n");
            //    //    continue;
            //    //}
            //}

            spdlog::info("Got valid property list(0x{:X}), saving to record.", (uintptr_t)property_list);
            // Create our property list
            std::vector<ProcessedProperty> props;
            if (property_list->mpElement != nullptr)
            {

                for(auto mt_prop = property_list->mpElement; mt_prop != nullptr && mt_prop->name() != nullptr; mt_prop = mt_prop->next())
                {
                    //spdlog::info("Property: {}", mt_prop->name());

                    // We have to treat the name as bytes because the json encoder
                    // only accepts utf8 characters.
                    auto ssize = strnlen_s(mt_prop->name(), 1024);
                    std::vector<uint8_t> name_bytes(ssize, 0);
                    memcpy_s(name_bytes.data(), name_bytes.size(), mt_prop->name(), ssize);

                    ProcessedProperty pp = {
                         .name_bytes = name_bytes,
                        //.name = std::string(mt_prop->name()),
                        .type = mt_prop->type(),
                        .attr = mt_prop->attr(),
                        .owner = mt_prop->owner(),
                        .get = mt_prop->get(),
                        .get_count = mt_prop->get_count(),
                        .set = mt_prop->set(),
                        .set_count = mt_prop->set_count(),
                        .index = mt_prop->index(),
                    };
                    props.push_back(pp);
                }
            }

            ProcessedVftable pv = {
                .vftable_va = class_record.class_vftables[vftable_idx],
                .properties = props,
            };
            m_class_records[i].properties.push_back(pv);
        }
        m_class_records[i].properties_processed = true;
    }
    
    spdlog::info("Done processing properties!");
}

} // namespace Dumper
