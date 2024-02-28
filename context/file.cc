#include <context/file.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <uv.h>

namespace file {

    #define buffer_length 1024

    void on_read(uv_fs_t *req);
    void on_write(uv_fs_t *req);
    void on_open(uv_fs_t *req);

    static uv_fs_t open_req;
    static uv_fs_t read_req;
    static uv_fs_t write_req;

    static char buffer[buffer_length];

    static uv_buf_t iov;

    static char* file_content;
    static int size = 0;

    static int isWriteMode = 0;

    void on_write(uv_fs_t *req) {
        if (req->result < 0) {
            fprintf(stderr, "Write error: %s\n", uv_strerror((int)req->result));
        }
        else {
            if(isWriteMode == 0){
                uv_fs_read(uv_default_loop(), &read_req, open_req.result, &iov, 1, -1, on_read);
            }
            else {
                uv_fs_t close_req;
                uv_fs_close(uv_default_loop(), &close_req, open_req.result, NULL);
            }
        }
    }

    void on_read(uv_fs_t *req) {
        if (req->result < 0) {
            fprintf(stderr, "Read error: %s\n", uv_strerror(req->result));
        }
        else if (req->result == 0) {
            uv_fs_t close_req;
            uv_fs_close(uv_default_loop(), &close_req, open_req.result, NULL);
        }
        else if (req->result > 0) {
            iov.len = req->result;
            file_content = (char *) realloc(file_content, size + req->result);
            // printf("--- %s\n\n --- ", buffer);
            memcpy(file_content + size, buffer, req->result);
            size = size + req->result;
            uv_fs_write(uv_default_loop(), &write_req, 1, &iov, 1, -1, on_write);
        }
    }

    void on_open(uv_fs_t *req) {
        // The request passed to the callback is the same as the one the call setup
        // function was passed.
        assert(req == &open_req);
        if (req->result >= 0) {
            if(isWriteMode == 0){
                iov = uv_buf_init(buffer, sizeof(buffer));
                uv_fs_read(uv_default_loop(), &read_req, req->result,
                        &iov, 1, -1, on_read);
            }
            else{

                iov = uv_buf_init((char*)req->data, strlen((char*)req->data));
                uv_fs_write(uv_default_loop(), &write_req, req->result,
                        &iov, 1, -1, on_write);
            }
        }
        else {
            fprintf(stderr, "error opening file: %s\n", uv_strerror((int)req->result));
        }
    }

    // Extracts a C string from a V8 Utf8Value.
    const char* ToCString(const v8::String::Utf8Value& value) {
        return *value ? *value : "<string conversion failed>";
    }

    void read_file(const char* name) {
        uv_loop_t* loop = uv_default_loop();
        uv_fs_open(uv_default_loop(), &open_req, name, O_RDONLY, 0, on_open);
        uv_run(loop, UV_RUN_DEFAULT);

        uv_fs_req_cleanup(&open_req);
        uv_fs_req_cleanup(&read_req);
        uv_fs_req_cleanup(&write_req);
    }

    void write_file(const char* name, const char* data) {
        uv_loop_t* loop = uv_default_loop();
        int flags = O_CREAT | O_WRONLY | O_TRUNC;
        int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; // File permissions
        isWriteMode = 1;
        open_req.data = (void*)data;
        uv_fs_open(uv_default_loop(), &open_req, name, flags, mode, on_open);

        uv_run(loop, UV_RUN_DEFAULT);

        uv_fs_req_cleanup(&open_req);
        uv_fs_req_cleanup(&read_req);
        uv_fs_req_cleanup(&write_req);
    }
    
    void Read(const v8::FunctionCallbackInfo<v8::Value>& info) {
        bool first = true;
        char* file_name;
        for (int i = 0; i < info.Length(); i++) {
            v8::HandleScope handle_scope(info.GetIsolate());
            v8::String::Utf8Value str(info.GetIsolate(), info[i]);
            const char* temp = ToCString(str);
            file_name = (char*)malloc(strlen(temp));
            strcpy(file_name, temp);
            // free((void*)temp);
        }

        read_file(file_name);

        v8::MaybeLocal<v8::String> result = v8::String::NewFromUtf8(
        info.GetIsolate(), file_content , v8::NewStringType::kNormal, static_cast<int>(size));
        delete[] file_content;
        free(file_name);
        v8::Local<v8::String> source;
        if (!result.ToLocal(&source)) {
                info.GetIsolate()->ThrowError("Error loading file");
                return;
        }
        info.GetReturnValue().Set(source);
    }

    void Write(const v8::FunctionCallbackInfo<v8::Value>& info) {
        char* file_name;
        char* incoming_data;
        for (int i = 0; i < info.Length(); i++) {
            v8::HandleScope handle_scope(info.GetIsolate());
            v8::String::Utf8Value str(info.GetIsolate(), info[i]);
            const char* temp = ToCString(str);
            if(i == 1){
                incoming_data = (char*)malloc(strlen(temp));
                strcpy(incoming_data, temp);
            }
            if(i == 0){
                file_name = (char*)malloc(strlen(temp));
                strcpy(file_name, temp);
            }
            // free((void*)temp);
        }

        write_file(file_name, incoming_data);

        // v8::MaybeLocal<v8::String> result = v8::String::NewFromUtf8(
        // info.GetIsolate(), file_content , v8::NewStringType::kNormal, static_cast<int>(size));
        // delete[] file_content;
        // free(file_name);
        // v8::Local<v8::String> source;
        // if (!result.ToLocal(&source)) {
        //         info.GetIsolate()->ThrowError("Error loading file");
        //         return;
        // }
        // info.GetReturnValue().Set(source);
    }

    void add(v8::Local<v8::ObjectTemplate> global, v8::Isolate* isolate) {

        v8::Local<v8::ObjectTemplate> file_object = v8::ObjectTemplate::New(isolate);
        file_object->Set(isolate, "read", v8::FunctionTemplate::New(isolate, Read));
        file_object->Set(isolate, "write", v8::FunctionTemplate::New(isolate, Write));

        global->Set(isolate, "file", file_object);
    }
}