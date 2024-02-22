#include <context/file.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


namespace file {

    // Extracts a C string from a V8 Utf8Value.
    const char* ToCString(const v8::String::Utf8Value& value) {
        return *value ? *value : "<string conversion failed>";
    }

    char* ReadFile(const char* name) {
        FILE* file = fopen(name, "rb");
        if (file == NULL) return NULL;

        fseek(file, 0, SEEK_END);
        size_t size = ftell(file);
        rewind(file);

        char* chars = new char[size + 1];
        chars[size] = '\0';
        for (size_t i = 0; i < size;) {
            i += fread(&chars[i], 1, size - i, file);
            if (ferror(file)) {
            fclose(file);
            return NULL;
            }
        }
        fclose(file);
        return chars;
    }


    void Read(const v8::FunctionCallbackInfo<v8::Value>& info) {
        bool first = true;
        char* file_name;
        for (int i = 0; i < info.Length(); i++) {
            v8::HandleScope handle_scope(info.GetIsolate());
            if (first) {
            first = false;
            } else {
            printf(" ");
            }
            v8::String::Utf8Value str(info.GetIsolate(), info[i]);
            const char* temp = ToCString(str);
            file_name = (char*)malloc(strlen(temp));
            strcpy(file_name, temp);
        }
        printf("%s", ReadFile(file_name));
        free(file_name);
    }

    void add(v8::Local<v8::ObjectTemplate> global, v8::Isolate* isolate) {

        v8::Local<v8::ObjectTemplate> file_object = v8::ObjectTemplate::New(isolate);
        file_object->Set(isolate, "read", v8::FunctionTemplate::New(isolate, Read));

        global->Set(isolate, "file", file_object);
    }
}