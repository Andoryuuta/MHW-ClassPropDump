#pragma once
#include <cstdint>

#include "mt_dti.h"
#include "mt_dti_hash_table.h"
#include "mt_object.h"
#include "mt_property_list.h"

#include "config.h"

namespace Mt
{
class MtPropertyList;

class cResource : MtObject
{
  public:
    virtual ~cResource() = 0;

    virtual void getUpdateTime() = 0;
    virtual const char* getExt() = 0;
    virtual void compact() = 0;
    virtual void create() = 0;
    virtual void loadEnd() = 0;
    virtual void load() = 0;
    virtual void save() = 0;
    virtual void convert() = 0;
    virtual void convertEx() = 0;
    virtual void clear() = 0;
};
} // namespace Mt

#if defined(MT_GAME_MHW)
    #include "mt/game/MHW.h"
#elif defined(MT_GAME_MHS2)
    #include "mt/game/MHS2.h"
#elif defined(MT_GAME_UMVC3)
    #include "mt/game/UMVC3.h"
#elif defined(MT_GAME_DDDA)
    #include "mt/game/DDDA.h"
#elif defined(MT_GAME_DDON)
    #include "mt/game/DDON.h"
#endif