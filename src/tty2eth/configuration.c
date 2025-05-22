#include "configuration.h"
#include "utils/assert.h"
#include "memfs.h"
#include "main.h"
#include "lwip.h"
#include "lwip/dns.h"
#include <string.h>

cJSON* mainConfig = NULL;
FileDef* mainConfigFile = NULL;
__attribute__((section(".storage"))) char mainConfigStorage[7*1000] = R""""(
{
    "auto-boot": true,
	"sshd":	{},

    "metrics": {
        "identifier": "tty2eth-test",
        "enable":true,
        "listenHTTPS": false
    },

    "network": {
        "dhcp": true,
    },

    "users": {
        "root" : {
            "no-auth" : true
        },
        "user1" : {
            "password": "2c26b46b68ffc68ff99b453c1d30413413422d706483bfa0f98a5e886266e7ae",
            "keys": []
        }
    }
}

)"""";


FileDef* metricCertFile = NULL;
__attribute__((section(".storage"))) char metricCertStorage[4*1000] = "";


FileDef* metricKeyFile = NULL;
__attribute__((section(".storage"))) char  metricKeyStorage[4*1000] = "";


void configUpdateMain(FileDef* file)
{
    if(mainConfig)
        cJSON_Delete(mainConfig);
    mainConfig = cJSON_ParseWithLength(file->start, file->fileSize);
    assert(mainConfig, "main config error, near pos: %d, context: %s", (size_t)(cJSON_GetErrorPtr() - (char*)file->start),  cJSON_GetErrorPtr());
}

void configParseNetwork()
{
    cJSON* dhcp = cJSONUtils_GetPointer(mainConfig, "/network/dhcp");
    cJSON* ip = cJSONUtils_GetPointer(mainConfig, "/network/ip");
    cJSON* gateway = cJSONUtils_GetPointer(mainConfig, "/network/gateway");
    cJSON* dns = cJSONUtils_GetPointer(mainConfig, "/network/dns");

    useDHCP = cJSON_IsTrue(dhcp);
    if(cJSON_IsString(ip))
    {
        char* ipStr = cJSON_GetStringValue(ip);
        char* netStr = strchr(ipStr, '/');
        if(assert(netStr, "no netmask part in network.ip is defined"))
            return;
        *netStr = 0;

        assert(ip4addr_aton(ipStr, &ipaddr), "cannot parse network.ip");
        netmask.addr = PP_HTONL(0xffffffff<<(32-atoi(netStr)));
    }
    if(cJSON_IsString(gateway))
        assert(ip4addr_aton(cJSON_GetStringValue(gateway), &gw), "cannot parse network.gateway");
    if(cJSON_IsString(dns))
    {
        ip_addr_t dnsIP = {0};
        assert(ip4addr_aton(cJSON_GetStringValue(dns), &dnsIP), "cannot parse network.dns");
        dns_setserver(0, &dnsIP);
    }
}

void initConfigFiles()
{
    mainConfigFile = memAllocateFile("/config.json", 
        mainConfigStorage, 
        strlen(mainConfigStorage), 
        sizeof(mainConfigStorage), true, configUpdateMain);
    configUpdateMain(mainConfigFile);

    metricCertFile = memAllocateFile("/metric-server.cet", 
        metricCertStorage, 
        strlen(metricCertStorage), 
        sizeof(metricCertStorage), true, NULL);
    metricKeyFile = memAllocateFile("/metric-server.key", 
        metricKeyStorage, 
        strlen(metricKeyStorage), 
        sizeof(metricKeyStorage), true, NULL);

    configParseNetwork();
}

bool configIsAutoBoot()
{
    cJSON* autoboot = cJSONUtils_GetPointer(mainConfig, "/auto-boot");
    return autoboot && cJSON_IsTrue(autoboot);
}

char* configGetMetricIdentifier()
{
    cJSON* identifier = cJSONUtils_GetPointer(mainConfig, "/metrics/identifier");
    if(cJSON_IsString(identifier))
        return cJSON_GetStringValue(identifier);
    return "";
}