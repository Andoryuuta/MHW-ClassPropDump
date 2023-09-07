#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace Mt
{
class MtDTI;
struct MtDTIHashTable;
} // namespace Mt

namespace Dumper
{
struct ProcessedProperty
{
  public:
    //std::string name;
    std::vector<uint8_t> name_bytes;
    uint32_t type;
    uint32_t attr;
    uintptr_t owner;
    uintptr_t get;
    uintptr_t get_count;
    uintptr_t set;
    uintptr_t set_count;
    uint32_t index;
};

struct ProcessedVftable
{
  public:
    uintptr_t vftable_va;
    std::vector<ProcessedProperty> properties;
};

struct ProcessedClassRecord
{
  public:
    uintptr_t image_base;
    uintptr_t dti;
    uintptr_t dti_va;
    uintptr_t dti_vftable_va;
    uint32_t id;
    std::string name;
    uint32_t parent_id;
    uint32_t raw_flags_and_size;
    uint32_t size;
    uint32_t allocator_index;
    bool is_abstract;
    bool is_hidden;
    bool properties_processed = false;
    std::vector<uintptr_t> class_vftables;
    std::vector<ProcessedVftable> properties;
};

class Dumper
{
  public:
    Dumper(uintptr_t image_base);
    ~Dumper();

    void Initialize();

    // Build class records from the DTI.
    void BuildClassRecords();
    // void LoadClassRecords(std::string_view filename);

    void DumpDTIMap(std::string_view filename);
    void DumpResourceInformation(std::string_view filename);

    void ProcessProperties();

    // void ParseDTI();
    // void DumpRawInfo(std::string filename);
    // void DumpToFile(std::string filename);
    // void DumpPythonArrayFile(std::string filename);
    // void DumpResourceInformation(std::string filename);
  private:
    std::vector<Mt::MtDTI*> GetSortedDTIVector();
    Mt::MtDTI* GetDTIByName(std::string_view name);

  private:
    uintptr_t m_image_base;
    Mt::MtDTIHashTable* m_dti_hash_table;
    std::vector<Mt::MtDTI*> m_sorted_dti_vec;
    std::vector<ProcessedClassRecord> m_class_records;
};
} // namespace Dumper
