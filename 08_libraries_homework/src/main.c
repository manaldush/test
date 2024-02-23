#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json.h>

#define URL_MAX_PATH_LENGTH 70

typedef struct body {
  char *response;
  size_t size;
} body;
 
static size_t cb(const void *data, size_t size, size_t nmemb, void *clientp)
{
    size_t realsize = size * nmemb;
    body *mem = (body *)clientp;
 
    char *ptr = realloc(mem->response, mem->size + realsize + 1);
    if(ptr == NULL) {
        fprintf(stderr, "Out if memory occured\n");
        return 0;  /* out of memory! */
    }
 
    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;
 
    return realsize;
}

int main(int argc, char** argv) {
    if (argc < 2) {
       fprintf(stderr, "Input parameters are missed\n");
       return EXIT_FAILURE;
    }

    const char* city = argv[1];

    char url[URL_MAX_PATH_LENGTH];
    snprintf(url, URL_MAX_PATH_LENGTH, "https://wttr.in/%s?format=j1", city);
    if ((strlen(url) + 1) == URL_MAX_PATH_LENGTH) {
        printf("Suspicious city value %s\n", city);
        return EXIT_FAILURE;
    }

    CURL *curl;
    CURLcode res;
    body data = {NULL, 0};
 
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&data);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
 
        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK || data.response == NULL)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
 
        /* always cleanup */
        curl_easy_cleanup(curl);

        struct json_object *jobj, *current_conditions, *current_condition, *weatherDescs, *weatherDesc,*attr;
        jobj = json_tokener_parse(data.response);

        json_object_object_get_ex(jobj, "current_condition", &current_conditions);
        current_condition = json_object_array_get_idx(current_conditions, 0);

        json_object_object_get_ex(current_condition, "temp_C", &attr);
        printf("temp C = %s\n", json_object_get_string(attr));

        json_object_object_get_ex(current_condition, "windspeedKmph", &attr);
        printf("windspeedKmph = %s\n", json_object_get_string(attr));

        json_object_object_get_ex(current_condition, "winddir16Point", &attr);
        printf("winddir16Point = %s\n", json_object_get_string(attr));


        json_object_object_get_ex(current_condition, "weatherDesc", &weatherDescs);
        weatherDesc = json_object_array_get_idx(weatherDescs, 0);
        json_object_object_get_ex(weatherDesc, "value", &attr);
        printf("Description = %s\n", json_object_get_string(attr));

    } else {
        return EXIT_FAILURE;
    }

    if (data.response != NULL) {
        free(data.response);
    }

    return EXIT_SUCCESS;
}