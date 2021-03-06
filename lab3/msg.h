#define PORT_LEN 22
#define ADDR_LEN 16
#define MAX_ENTRIES 20

struct pairs
{
    char ip_address[ADDR_LEN];
    char port_number[PORT_LEN];
};

struct msg
{
    char command;
    struct pairs entry[MAX_ENTRIES];
};
