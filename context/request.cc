#include <context/file.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

namespace request {

    // Extracts a C string from a V8 Utf8Value.
    const char* ToCString(const v8::String::Utf8Value& value) {
        return *value ? *value : "<string conversion failed>";
    }


    static size_t writeToString(void *data, size_t size, size_t nmemb, void *userp)
    {
        size_t realsize = size * nmemb;
        std::string *str = static_cast<std::string*>(userp);
        str->append(static_cast<char*>(data), realsize);
        return realsize;
    }

    void Delete(const v8::FunctionCallbackInfo<v8::Value>& info) {
        CURL *curl;
        CURLcode res;
        char* url;
        char* body;

        for (int i = 0; i < info.Length(); i++) {
            v8::HandleScope handle_scope(info.GetIsolate());
            v8::String::Utf8Value str(info.GetIsolate(), info[i]);
            const char* temp = ToCString(str);
            if(i == 0) {
                url = (char*)malloc(strlen(temp));
                strcpy(url, temp);
            }

            if(i == 1) {
                body = (char*)malloc(strlen(temp));
                strcpy(body, temp);
            }
        }

        curl_global_init(CURL_GLOBAL_ALL); 
        curl = curl_easy_init();

        std::string respStr;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeToString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respStr);

        if(curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE"); 
        
            res = curl_easy_perform(curl);
            if(res != CURLE_OK)
            fprintf(stderr, "put() failed: %s\n",
                    curl_easy_strerror(res));
        
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();

        v8::MaybeLocal<v8::String> result = v8::String::NewFromUtf8(info.GetIsolate(), respStr.c_str());
        v8::Local<v8::String> source;
        if (!result.ToLocal(&source)) {
                info.GetIsolate()->ThrowError("Error while put");
                return;
        }
        info.GetReturnValue().Set(source);
    }
    
    void Put(const v8::FunctionCallbackInfo<v8::Value>& info) {
        CURL *curl;
        CURLcode res;
        char* url;
        char* body;

        for (int i = 0; i < info.Length(); i++) {
            v8::HandleScope handle_scope(info.GetIsolate());
            v8::String::Utf8Value str(info.GetIsolate(), info[i]);
            const char* temp = ToCString(str);
            if(i == 0) {
                url = (char*)malloc(strlen(temp));
                strcpy(url, temp);
            }

            if(i == 1) {
                body = (char*)malloc(strlen(temp));
                strcpy(body, temp);
            }
        }

        curl_global_init(CURL_GLOBAL_ALL); 
        curl = curl_easy_init();

        std::string respStr;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeToString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respStr);

        if(curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT"); 
        
            res = curl_easy_perform(curl);
            if(res != CURLE_OK)
            fprintf(stderr, "put() failed: %s\n",
                    curl_easy_strerror(res));
        
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();

        v8::MaybeLocal<v8::String> result = v8::String::NewFromUtf8(info.GetIsolate(), respStr.c_str());
        v8::Local<v8::String> source;
        if (!result.ToLocal(&source)) {
                info.GetIsolate()->ThrowError("Error while put");
                return;
        }
        info.GetReturnValue().Set(source);
    }
    
    void Post(const v8::FunctionCallbackInfo<v8::Value>& info) {

        CURL *curl;
        CURLcode res;
        char* url;
        char* body;

        for (int i = 0; i < info.Length(); i++) {
            v8::HandleScope handle_scope(info.GetIsolate());
            v8::String::Utf8Value str(info.GetIsolate(), info[i]);
            const char* temp = ToCString(str);
            if(i == 0) {
                url = (char*)malloc(strlen(temp));
                strcpy(url, temp);
            }

            if(i == 1) {
                body = (char*)malloc(strlen(temp));
                strcpy(body, temp);
            }
        }

        curl_global_init(CURL_GLOBAL_ALL); 
        curl = curl_easy_init();

        std::string respStr;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeToString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respStr);

        if(curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
        
            res = curl_easy_perform(curl);
            if(res != CURLE_OK)
            fprintf(stderr, "post() failed: %s\n",
                    curl_easy_strerror(res));
        
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();

        v8::MaybeLocal<v8::String> result = v8::String::NewFromUtf8(info.GetIsolate(), respStr.c_str());
        v8::Local<v8::String> source;
        if (!result.ToLocal(&source)) {
                info.GetIsolate()->ThrowError("Error while post");
                return;
        }
        info.GetReturnValue().Set(source);
    }

    void Get(const v8::FunctionCallbackInfo<v8::Value>& info) {
        CURL *curl;
        CURLcode res;

        curl = curl_easy_init();

        char* url;
        for (int i = 0; i < info.Length(); i++) {
            v8::HandleScope handle_scope(info.GetIsolate());
            v8::String::Utf8Value str(info.GetIsolate(), info[i]);
            const char* temp = ToCString(str);
            if(i == 0) {
                url = (char*)malloc(strlen(temp));
                strcpy(url, temp);
            }
        }

        std::string respStr;
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeToString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &respStr);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
                fprintf(stderr, "get() failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);


        v8::MaybeLocal<v8::String> result = v8::String::NewFromUtf8(info.GetIsolate(), respStr.c_str());
        v8::Local<v8::String> source;
        if (!result.ToLocal(&source)) {
                info.GetIsolate()->ThrowError("Error while get");
                return;
        }
        info.GetReturnValue().Set(source);

    }

    void add(v8::Local<v8::ObjectTemplate> global, v8::Isolate* isolate) {

        v8::Local<v8::ObjectTemplate> file_object = v8::ObjectTemplate::New(isolate);
        file_object->Set(isolate, "get", v8::FunctionTemplate::New(isolate, Get));
        file_object->Set(isolate, "post", v8::FunctionTemplate::New(isolate, Post));
        file_object->Set(isolate, "put", v8::FunctionTemplate::New(isolate, Put));
        file_object->Set(isolate, "delete", v8::FunctionTemplate::New(isolate, Delete));

        global->Set(isolate, "request", file_object);
    }
}