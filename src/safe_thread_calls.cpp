// header
#include "safe_thread_calls.h"

// std
#include <windows.h>

// third-party libs
#include "spdlog/spdlog.h"


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
    bool got_past_native_call;
};

DWORD WINAPI try_dti_create_property_threaded_entry(LPVOID lpvParam)
{
    auto data = (try_dti_create_property_threaded_entry_struct*)lpvParam;
    data->result = try_dti_create_property(data->object, data->property_list);
    data->got_past_native_call = true;

    // Thread finished successfully.
    return 0;
}

bool try_dti_create_property_threaded(Mt::MtObject* object, Mt::MtPropertyList* property_list, uint32_t timeoutMs)
{
    // Create a large buffer on the heap to use for our thread-passing data.
    // This is required as the function we call may corrupt data around this pointer
    // if we don't have a proper vftable.
    std::vector<uint8_t> buf(1024*512, 0);
    auto data = reinterpret_cast<try_dti_create_property_threaded_entry_struct*>(buf.data());
    data->object = object;
    data->property_list = property_list;
    data->got_past_native_call = false;

    auto handle = CreateThread(NULL, 0, try_dti_create_property_threaded_entry, (LPVOID)data, 0, NULL);
    auto wait_result = WaitForSingleObject(handle, timeoutMs);
    if (wait_result == WAIT_TIMEOUT)
    {
        TerminateThread(handle, 1);
        spdlog::info("try_dti_create_property thread timed out/failed - terminating thread.");
        return false;
    }

    // In case we didn't time out, but didn't finish running our code.
    // (E.g. control flow went somewhere else that did a RET to finish the thread)
    if (data->got_past_native_call == false)
    {
        spdlog::info("try_dti_create_property failed.");
        return false;
    }

    // True if native call finished without an exception, else false.
    return data->result;
}

// Mt::MtObject* try_dti_create_new_instance(Mt::MtDTI* dti)
// {
//     __try
//     {
//         Mt::MtObject* obj = reinterpret_cast<Mt::MtObject*>(dti->NewInstance());
//         return obj;
//     }
//     __except (EXCEPTION_EXECUTE_HANDLER)
//     {
//         return nullptr;
//     }
// }

// struct try_dti_create_new_instance_threaded_entry_struct {
//     Mt::MtDTI* dti;
//     Mt::MtObject* result;
//     bool got_past_native_call;
// };

// DWORD WINAPI try_dti_create_new_instance_threaded_entry(LPVOID lpvParam)
// {
//     auto data = (try_dti_create_new_instance_threaded_entry_struct*)lpvParam;
//     data->result = try_dti_create_new_instance(data->dti);
//     data->got_past_native_call = true;
//     return 0;
// }

// Mt::MtObject* try_dti_create_new_instance_threaded(Mt::MtDTI* dti)
// {
//     // Create a large buffer on the heap to use for our thread-passing data.
//     // This is required as the function we call may corrupt data around this pointer
//     // if we don't have a proper vftable.
//     std::vector<uint8_t> buf(1024*128, 0);
//     auto data = reinterpret_cast<try_dti_create_new_instance_threaded_entry_struct*>(buf.data());
//     data->dti = dti;
//     data->result = nullptr;
//     data->got_past_native_call = false;

//     auto handle = CreateThread(NULL, 0, try_dti_create_new_instance_threaded_entry, (LPVOID)data, 0, NULL);
//     auto wait_result = WaitForSingleObject(handle, 1000);
//     if (wait_result == WAIT_TIMEOUT)
//     {
//         TerminateThread(handle, 1);
//         spdlog::info("try_dti_create_new_instance thread timed out/failed - terminating thread.");
//         return nullptr;
//     }

//     if (!data->got_past_native_call)
//     {
//         spdlog::info("try_dti_create_new_instance failed.");
//         return nullptr;
//     }

//     return data->result;
// }