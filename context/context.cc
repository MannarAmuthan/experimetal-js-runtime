#include <context/context.h>
#include <context/file.h>


namespace context {


void Print(const v8::FunctionCallbackInfo<v8::Value>& info);
void Read(const v8::FunctionCallbackInfo<v8::Value>& info);
void Load(const v8::FunctionCallbackInfo<v8::Value>& info);
void Quit(const v8::FunctionCallbackInfo<v8::Value>& info);
void Version(const v8::FunctionCallbackInfo<v8::Value>& info);
void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch);


// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

// Executes a string within the current v8 context.
bool ExecuteString(v8::Isolate* isolate, v8::Local<v8::String> source,
                   v8::Local<v8::Value> name, bool print_result,
                   bool report_exceptions) {
  v8::HandleScope handle_scope(isolate);
  v8::TryCatch try_catch(isolate);
  v8::ScriptOrigin origin(isolate, name);
  v8::Local<v8::Context> context(isolate->GetCurrentContext());
  v8::Local<v8::Script> script;
  if (!v8::Script::Compile(context, source, &origin).ToLocal(&script)) {
    // Print errors that happened during compilation.
    if (report_exceptions)
      ReportException(isolate, &try_catch);
    return false;
  } else {
    v8::Local<v8::Value> result;
    if (!script->Run(context).ToLocal(&result)) {
      assert(try_catch.HasCaught());
      // Print errors that happened during execution.
      if (report_exceptions)
        ReportException(isolate, &try_catch);
      return false;
    } else {
      assert(!try_catch.HasCaught());
      if (print_result && !result->IsUndefined()) {
        // If all went well and the result wasn't undefined then print
        // the returned value.
        v8::String::Utf8Value str(isolate, result);
        const char* cstr = ToCString(str);
        printf("%s\n", cstr);
      }
      return true;
    }
  }
}

v8::MaybeLocal<v8::String> ReadFile(v8::Isolate* isolate, const char* name) {
  FILE* file = fopen(name, "rb");
  if (file == NULL) return v8::MaybeLocal<v8::String>();

  fseek(file, 0, SEEK_END);
  size_t size = ftell(file);
  rewind(file);

  char* chars = new char[size + 1];
  chars[size] = '\0';
  for (size_t i = 0; i < size;) {
    i += fread(&chars[i], 1, size - i, file);
    if (ferror(file)) {
      fclose(file);
      return v8::MaybeLocal<v8::String>();
    }
  }
  fclose(file);
  v8::MaybeLocal<v8::String> result = v8::String::NewFromUtf8(
      isolate, chars, v8::NewStringType::kNormal, static_cast<int>(size));
  delete[] chars;
  return result;
}


// The callback that is invoked by v8 whenever the JavaScript 'print'
// function is called.  Prints its arguments on stdout separated by
// spaces and ending with a newline.
void Print(const v8::FunctionCallbackInfo<v8::Value>& info) {
  bool first = true;
  printf("len: %d", info.Length());
  for (int i = 0; i < info.Length(); i++) {
    v8::HandleScope handle_scope(info.GetIsolate());
    if (first) {
      first = false;
    } else {
      printf(" ");
    }
    v8::String::Utf8Value str(info.GetIsolate(), info[i]);
    const char* cstr = ToCString(str);
    printf("%s", cstr);
  }
  printf("\n");
  fflush(stdout);
}

// The callback that is invoked by v8 whenever the JavaScript 'read'
// function is called.  This function loads the content of the file named in
// the argument into a JavaScript string.
void Read(const v8::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() != 1) {
    info.GetIsolate()->ThrowError("Bad parameters");
    return;
  }
  v8::String::Utf8Value file(info.GetIsolate(), info[0]);
  if (*file == NULL) {
    info.GetIsolate()->ThrowError("Error loading file");
    return;
  }
  v8::Local<v8::String> source;
  if (!ReadFile(info.GetIsolate(), *file).ToLocal(&source)) {
    info.GetIsolate()->ThrowError("Error loading file");
    return;
  }

  info.GetReturnValue().Set(source);
}

// The callback that is invoked by v8 whenever the JavaScript 'load'
// function is called.  Loads, compiles and executes its argument
// JavaScript file.
void Load(const v8::FunctionCallbackInfo<v8::Value>& info) {
  for (int i = 0; i < info.Length(); i++) {
    v8::HandleScope handle_scope(info.GetIsolate());
    v8::String::Utf8Value file(info.GetIsolate(), info[i]);
    if (*file == NULL) {
      info.GetIsolate()->ThrowError("Error loading file");
      return;
    }
    v8::Local<v8::String> source;
    if (!ReadFile(info.GetIsolate(), *file).ToLocal(&source)) {
      info.GetIsolate()->ThrowError("Error loading file");
      return;
    }
    if (!ExecuteString(info.GetIsolate(), source, info[i], false, false)) {
      info.GetIsolate()->ThrowError("Error executing file");
      return;
    }
  }
}

// The callback that is invoked by v8 whenever the JavaScript 'quit'
// function is called.  Quits.
void Quit(const v8::FunctionCallbackInfo<v8::Value>& info) {
  // If not arguments are given info[0] will yield undefined which
  // converts to the integer value 0.
  int exit_code =
      info[0]->Int32Value(info.GetIsolate()->GetCurrentContext()).FromMaybe(0);
  fflush(stdout);
  fflush(stderr);
  exit(exit_code);
}

void Version(const v8::FunctionCallbackInfo<v8::Value>& info) {
  info.GetReturnValue().Set(
      v8::String::NewFromUtf8(info.GetIsolate(), v8::V8::GetVersion())
          .ToLocalChecked());
}


v8::Local<v8::Context> get_context(v8::Isolate* isolate) {
  // Create a template for the global object.
  v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);
  // Bind the global 'print' function to the C++ Print callback.
  global->Set(isolate, "print", v8::FunctionTemplate::New(isolate, Print));
  // Bind the global 'read' function to the C++ Read callback.
  global->Set(isolate, "read", v8::FunctionTemplate::New(isolate, Read));
  // Bind the global 'load' function to the C++ Load callback.
  global->Set(isolate, "load", v8::FunctionTemplate::New(isolate, Load));
  // Bind the 'quit' function
  global->Set(isolate, "quit", v8::FunctionTemplate::New(isolate, Quit));
  // Bind the 'version' function
  global->Set(isolate, "version", v8::FunctionTemplate::New(isolate, Version));

  file::add(global, isolate);
  return v8::Context::New(isolate, NULL, global);
}


void ReportException(v8::Isolate* isolate, v8::TryCatch* try_catch) {
  v8::HandleScope handle_scope(isolate);
  v8::String::Utf8Value exception(isolate, try_catch->Exception());
  const char* exception_string = ToCString(exception);
  v8::Local<v8::Message> message = try_catch->Message();
  if (message.IsEmpty()) {
    // V8 didn't provide any extra information about this error; just
    // print the exception.
    fprintf(stderr, "%s\n", exception_string);
  } else {
    // Print (filename):(line number): (message).
    v8::String::Utf8Value filename(isolate,
                                   message->GetScriptOrigin().ResourceName());
    v8::Local<v8::Context> context(isolate->GetCurrentContext());
    const char* filename_string = ToCString(filename);
    int linenum = message->GetLineNumber(context).FromJust();
    fprintf(stderr, "%s:%i: %s\n", filename_string, linenum, exception_string);
    // Print line of source code.
    v8::String::Utf8Value sourceline(
        isolate, message->GetSourceLine(context).ToLocalChecked());
    const char* sourceline_string = ToCString(sourceline);
    fprintf(stderr, "%s\n", sourceline_string);
    // Print wavy underline (GetUnderline is deprecated).
    int start = message->GetStartColumn(context).FromJust();
    for (int i = 0; i < start; i++) {
      fprintf(stderr, " ");
    }
    int end = message->GetEndColumn(context).FromJust();
    for (int i = start; i < end; i++) {
      fprintf(stderr, "^");
    }
    fprintf(stderr, "\n");
    v8::Local<v8::Value> stack_trace_string;
    if (try_catch->StackTrace(context).ToLocal(&stack_trace_string) &&
        stack_trace_string->IsString() &&
        stack_trace_string.As<v8::String>()->Length() > 0) {
      v8::String::Utf8Value stack_trace(isolate, stack_trace_string);
      const char* err = ToCString(stack_trace);
      fprintf(stderr, "%s\n", err);
    }
  }
}
}
