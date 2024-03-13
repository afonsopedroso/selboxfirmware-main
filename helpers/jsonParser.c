#include <stdio.h>
#include <json-c/json.h>

int main(int argc, char **argv)
{
    FILE *fp;
    char buffer[1024];
    struct json_object *parsed_json;
    struct json_object *version;
    struct json_object *url;
    size_t n_friends;

    size_t i;

    fp = fopen("test.json", "r");
    fread(buffer, 1024, 1, fp);
    fclose(fp);

    printf("%s\n", buffer);
    parsed_json = json_tokener_parse(buffer);

    json_object_object_get_ex(parsed_json, "version", &version);
    json_object_object_get_ex(parsed_json, "url", &url);

    printf("Version: %s\n", json_object_get_string(version));
    printf("URL: %s\n", json_object_get_string(url));
}