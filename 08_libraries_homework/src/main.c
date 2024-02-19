#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

int main(int argc, char** argv) {
    if (argc < 2) {
       fprintf(stderr, "Input parameters are missed\n");
       return EXIT_FAILURE;
    }

    const char* city = argv[1];

    CURL *curl;
    CURLcode res;
 
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://wttr.in/Moscow?format=j1");
        /* example.com is redirected, so we tell libcurl to follow redirection */
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
 
        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
 
        /* always cleanup */
        curl_easy_cleanup(curl);
    } else {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}