#include <assert.h>
#include <fcntl.h>
#include <libplatform/libplatform.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "v8-context.h"
#include "v8-exception.h"
#include "v8-initialization.h"
#include "v8-isolate.h"
#include "v8-local-handle.h"
#include "v8-script.h"
#include "v8-template.h"


#include <yappu.h>

/**
 * This sample program shows how to implement a simple javascript shell
 * based on V8.  This includes initializing V8 with command line options,
 * creating global functions, compiling and executing strings.
 *
 * For a more sophisticated shell, consider using the debug shell D8.
 */


v8::Local<v8::Context> CreateShellContext(v8::Isolate* isolate);
void RunShell(v8::Local<v8::Context> context, v8::Platform* platform);
int RunMain(v8::Isolate* isolate, v8::Platform* platform, int argc,
            char* argv[]);
bool ExecuteString(v8::Isolate* isolate, v8::Local<v8::String> source,
                   v8::Local<v8::Value> name, bool print_result,
                   bool report_exceptions);
void Print(const v8::FunctionCallbackInfo<v8::Value>& info);
void Read(const v8::FunctionCallbackInfo<v8::Value>& info);
void Load(const v8::FunctionCallbackInfo<v8::Value>& info);
void Quit(const v8::FunctionCallbackInfo<v8::Value>& info);
void Version(const v8::FunctionCallbackInfo<v8::Value>& info);
v8::MaybeLocal<v8::String> ReadFile(v8::Isolate* isolate, const char* name);
void ReportException(v8::Isolate* isolate, v8::TryCatch* handler);


static bool run_shell;


int main(int argc, char* argv[]) {
  v8::V8::InitializeICUDefaultLocation(argv[0]);
  v8::V8::InitializeExternalStartupData(argv[0]);
  std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
  v8::V8::InitializePlatform(platform.get());
  v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
  v8::V8::Initialize();
  v8::Isolate::CreateParams create_params;
  create_params.array_buffer_allocator =
      v8::ArrayBuffer::Allocator::NewDefaultAllocator();
  v8::Isolate* isolate = v8::Isolate::New(create_params);
  run_shell = (argc == 1);
  int result;
  {
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Context> context = context::get_context(isolate);
    if (context.IsEmpty()) {
      fprintf(stderr, "Error creating context\n");
      return 1;
    }
    v8::Context::Scope context_scope(context);
    result = RunMain(isolate, platform.get(), argc, argv);
    if (run_shell) RunShell(context, platform.get());
  }
  isolate->Dispose();
  v8::V8::Dispose();
  v8::V8::DisposePlatform();
  delete create_params.array_buffer_allocator;
  return result;
}


// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

// Reads a file into a v8 string.
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


// Process remaining command line arguments and execute files
int RunMain(v8::Isolate* isolate, v8::Platform* platform, int argc,
            char* argv[]) {
  for (int i = 1; i < argc; i++) {
    const char* str = argv[i];
    if (strcmp(str, "--shell") == 0) {
      run_shell = true;
    } else if (strcmp(str, "-f") == 0) {
      // Ignore any -f flags for compatibility with the other stand-
      // alone JavaScript engines.
      continue;
    } else if (strncmp(str, "--", 2) == 0) {
      fprintf(stderr,
              "Warning: unknown flag %s.\nTry --help for options\n", str);
    } else if (strcmp(str, "-e") == 0 && i + 1 < argc) {
      // Execute argument given to -e option directly.
      v8::Local<v8::String> file_name =
          v8::String::NewFromUtf8Literal(isolate, "unnamed");
      v8::Local<v8::String> source;
      if (!v8::String::NewFromUtf8(isolate, argv[++i]).ToLocal(&source)) {
        return 1;
      }
      bool success = ExecuteString(isolate, source, file_name, false, true);
      while (v8::platform::PumpMessageLoop(platform, isolate)) continue;
      if (!success) return 1;
    } else {
      // Use all other arguments as names of files to load and run.
      v8::Local<v8::String> file_name =
          v8::String::NewFromUtf8(isolate, str).ToLocalChecked();
      v8::Local<v8::String> source;
      if (!ReadFile(isolate, str).ToLocal(&source)) {
        fprintf(stderr, "Error reading '%s'\n", str);
        continue;
      }
      bool success = ExecuteString(isolate, source, file_name, false, true);
      while (v8::platform::PumpMessageLoop(platform, isolate)) continue;
      if (!success) return 1;
    }
  }
  return 0;
}


// The read-eval-execute loop of the shell.
void RunShell(v8::Local<v8::Context> context, v8::Platform* platform) {
  fprintf(stderr, "Yappu version v0\n");
  static const int kBufferSize = 256;
  // Enter the execution environment before evaluating any code.
  v8::Context::Scope context_scope(context);
  v8::Local<v8::String> name(
      v8::String::NewFromUtf8Literal(context->GetIsolate(), "(shell)"));
  while (true) {
    char buffer[kBufferSize];
    fprintf(stderr, "> ");
    char* str = fgets(buffer, kBufferSize, stdin);
    if (str == NULL) break;
    v8::HandleScope handle_scope(context->GetIsolate());
    ExecuteString(
        context->GetIsolate(),
        v8::String::NewFromUtf8(context->GetIsolate(), str).ToLocalChecked(),
        name, true, true);
    while (v8::platform::PumpMessageLoop(platform, context->GetIsolate()))
      continue;
  }
  fprintf(stderr, "\n");
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
