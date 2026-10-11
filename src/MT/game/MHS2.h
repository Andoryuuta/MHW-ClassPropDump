#pragma once
// Monster Hunter Stories 2: Wings of Ruin (1.5.3.0)

// OEP: RVA: 14491AC
// 00000001414491AC | 48:83EC 28               | sub rsp,28 | IAT start:
// 000000014153B000

namespace Mt
{
class MtProperty;
class MtPropertyList
{
  public:
    virtual ~MtPropertyList() = 0;

    uint64_t field_8;
    MtProperty* mpElement;
};
assert_size_64(MtPropertyList, 0x18);
} // namespace Mt
