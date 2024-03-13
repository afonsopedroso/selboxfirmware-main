#include "config.h"

int main(void)
{
    config();
    return 0;
}

int config(void)
{
    FILE *fp;
    char buffer[1024];
    struct json_object *parsed_json;
    struct json_object *jdeviceid;
    struct json_object *jfirmware;
    struct json_object *jhardware;
    // struct json_object *jvoltrating;
    // struct json_object *jloggedpts;
    // struct json_object *jvoltagecal;
    // struct json_object *jcurrentcal;
    // struct json_object *jcha_offcal;
    // struct json_object *jchb_offcal;

    fp = fopen("/home/sel/device.conf", "r");
    fread(buffer, 1024, 1, fp);
    fclose(fp);

    // printf("%s\n", buffer);
    parsed_json = json_tokener_parse(buffer);

    json_object_object_get_ex(parsed_json, "deviceid", &jdeviceid);
    json_object_object_get_ex(parsed_json, "firmware", &jfirmware);
    json_object_object_get_ex(parsed_json, "hardware", &jhardware);
    // json_object_object_get_ex(parsed_json, "ctrating", &jctrating);
    //  json_object_object_get_ex(parsed_json, "voltrating", &jvoltrating);
    //  json_object_object_get_ex(parsed_json, "loggedpts", &jloggedpts);
    //  json_object_object_get_ex(parsed_json, "voltagecal", &jvoltagecal);
    //  json_object_object_get_ex(parsed_json, "currentcal", &jcurrentcal);
    //  json_object_object_get_ex(parsed_json, "cha_offcal", &jcha_offcal);
    //  json_object_object_get_ex(parsed_json, "chb_offcal", &jchb_offcal);

    printf("----------------------------------\n");
    printf("\tDefault Variables\n");
    printf("----------------------------------\n");
    printf("Device ID (MAC Address)\t\t: %s\n", mac);
    printf("Firmware Version \t\t: %f\n", firmware);
    printf("Firmware Version \t\t: %f\n", hardware);
    // printf("Voltage Rating \t\t\t: %d\n", voltRating);
    // printf("Number of Logged Data Points \t: %d\n", loggedPoints);
    // printf("Voltage Calibration Constant \t: %.3f\n", voltResComp);
    // printf("Current Calibration Constant \t: %.3f\n", ctResComp);
    // printf("Ch. A Offset Calibration Value\t: %.3f\n", offsetVoltage);
    // printf("Ch. B Offset Calibration Value\t: %.3f\n", offsetCurrent);

    printf("----------------------------------\n");
    printf("\tRead Variables\n");
    printf("----------------------------------\n");
    printf("Device ID (MAC Address)\t\t: %s\n", json_object_get_string(jdeviceid));
    printf("Firmware version \t\t: %s\n", json_object_get_string(jfirmware));
    printf("Hardware version \t\t: %s\n", json_object_get_string(jhardware));
    // printf("Voltage Rating \t\t\t: %s\n", json_object_get_string(jvoltrating));
    // printf("CT Current Rating \t\t: %s\n", json_object_get_string(jctrating));
    // printf("Number of Logged  Data Points \t: %s\n", json_object_get_string(jloggedpts));
    // printf("Voltage Calibration Constant \t: %s\n", json_object_get_string(jvoltagecal));
    // printf("Current Calibration Constant \t: %s\n", json_object_get_string(jcurrentcal));
    // printf("Ch. A Offset Calibration Value\t: %s\n", json_object_get_string(jcha_offcal));
    // printf("Ch. B Offset Calibration Value\t: %s\n", json_object_get_string(jchb_offcal));

    strcpy(mac, json_object_get_string(jdeviceid));
    firmware = atof(json_object_get_string(jfirmware));
    hardware = atof(json_object_get_string(jhardware));
    // voltRating = atoi(json_object_get_string(jvoltrating));
    // loggedPoints = atoi(json_object_get_string(jloggedpts));
    // voltResComp = atof(json_object_get_string(jvoltagecal));
    // ctResComp = atof(json_object_get_string(jcurrentcal));
    // offsetVoltage = atof(json_object_get_string(jcha_offcal));
    // offsetCurrent = atof(json_object_get_string(jchb_offcal));

    printf("----------------------------------\n");
    printf("\tInternal Variables\n");
    printf("----------------------------------\n");
    printf("Device ID (MAC Address)\t\t: %s\n", mac);
    printf("Firmware Version \t\t: %f\n", firmware);
    printf("Hardware Version \t\t: %f\n", hardware);
    printf("CT Current Rating \t\t: %d\n", ctRating);
    // printf("Voltage Rating \t\t\t: %d\n", voltRating);
    // printf("Number of Logged  Data Points \t: %d\n", loggedPoints);
    // printf("Voltage Calibration Constant \t: %.3f\n", voltResComp);
    // printf("Current Calibration Constant \t: %.3f\n", ctResComp);
    // printf("Ch. A Offset Calibration Value\t: %.3f\n", offsetVoltage);
    // printf("Ch. B Offset Calibration Value\t: %.3f\n", offsetCurrent);

    // overwrite file with new json format string
    fp = fopen("/home/sel/device.conf", "w");
    if (strcmp(mac, "b827eb6b4e40") == 0)
    {
        ctRating = 120;
    }
    else
    {
        ctRating = 30;
    }
    hardware = 0.1;

    sprintf(buffer, "{\"deviceid\" : \"%s\", \"firmware\" : \"%0.2f\", \"hardware\" : \"%0.2f\", \"ctrating\" : \"%d\"}", mac, firmware, hardware, ctRating);
    printf("%s\n", buffer);
    fputs(buffer, fp);
    fclose(fp);

    return 0;
}

// void getMacAddr(void)
// {
//     int fdMAC;
//     struct ifreq ifr;
//     char *iface = "wlan0";
//     fdMAC = socket(AF_INET, SOCK_DGRAM, 0);
//     ifr.ifr_addr.sa_family = AF_INET;
//     strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
//     ioctl(fdMAC, SIOCGIFHWADDR, &ifr);
//     close(fdMAC);
//     sprintf(mac, "%.2x%.2x%.2x%.2x%.2x%.2x",
//             (unsigned char)ifr.ifr_hwaddr.sa_data[0],
//             (unsigned char)ifr.ifr_hwaddr.sa_data[1],
//             (unsigned char)ifr.ifr_hwaddr.sa_data[2],
//             (unsigned char)ifr.ifr_hwaddr.sa_data[3],
//             (unsigned char)ifr.ifr_hwaddr.sa_data[4],
//             (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
// }