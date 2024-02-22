#ifndef YAPPU_CONTEXT_H_
#define YAPPU_CONTEXT_H_

#include <libplatform/libplatform.h>


#include "v8-context.h"
#include "v8-exception.h"
#include "v8-initialization.h"
#include "v8-isolate.h"
#include "v8-local-handle.h"
#include "v8-script.h"
#include "v8-template.h"

#include <assert.h>

namespace context{
    v8::Local<v8::Context> get_context(v8::Isolate* isolate);
}
#endif