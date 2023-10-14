#pragma once
#include <cstdint>

#include "config.h"
#include "mt_dti.h"
#include "mt_dti_hash_table.h"
#include "mt_object.h"

namespace Mt
{
class cResource : MtObject
{
  public:
    virtual ~cResource() = 0;

    /*
    // MtObject:
    virtual void* CreateUI() = 0;
    virtual bool IsEnableInstance() = 0;
    virtual void CreateProperty(MtPropertyList*) = 0;
    virtual MtDTI* GetDTI() = 0;
    */

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