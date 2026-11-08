#include "DNS.h"


unsigned char* getMessage(struct DNSHeader* header, struct DNSQuery* query, const unsigned char* message, int* len_header_query)
{
    char* ptr = message;
    int i, flag, num = 0;
    len_header_query = 0;

    header->id = ntohs(*((unsigned short*)ptr));
    ptr += 2;
    header->tag = ntohs(*((unsigned short*)ptr));
    ptr += 2;
    header->queryNum = ntohs(*((unsigned short*)ptr));
    ptr += 2;
    header->answerNum = ntohs(*((unsigned short*)ptr));
    ptr += 2;
    header->authorNum = ntohs(*((unsigned short*)ptr));
    ptr += 2;
    header->addNum = ntohs(*((unsigned short*)ptr));
    ptr += 2;
    len_header_query += 12;

    for(i = 0; i < header->queryNum; i++)
    {
        for(;;)
        {
            flag = (int)ptr[0];
            num += (flag + 1);
            ptr += (flag + 1);
            if(flag == 0)
                break;
        }
        ptr -= num ;
        memset(query->name, 0, sizeof(query->name));
        memcpy(query->name, ptr, num  - 1);
        ptr += num;
        len_header_query += num;
        query->qtype = ntohs(*((unsigned short*)ptr));
        ptr += 2;
        query->qclass = ntohs(*((unsigned short*)ptr));
        len_header_query += 4;
    }
    return (ptr + 2);
}

unsigned char* getRR(struct DNSRR rr[10], struct DNSHeader* header, unsigned char* ptr)
{
    int i, flag, num;
    int n;
    n = header->answerNum + header->authorNum;
    for(i = 0; i < n; i++)
    {
        num = 0;
        for(;;)
        {
            flag = (int)ptr[0];
            num += (flag + 1);
            ptr += (flag + 1);
            if(flag == 0)
                break;
        }
        ptr -= num;
        memset(rr[i].name, 0, sizeof(rr[i].name));
        memcpy(rr[i].name, ptr, num - 1);
        ptr += num;
        rr[i].type = ntohs(*((unsigned short*)ptr));
        ptr += 2;
        rr[i]._class = ntohs(*((unsigned short*)ptr));
        ptr += 2;
        rr[i].ttl = ntohl(*((unsigned short*)ptr));
        ptr += 4;
        rr[i].data_len = ntohs(*((unsigned short*)ptr));
        ptr += 2;
        if(rr[i].type == 2 || rr[i].type == 5)
            ptr++;
        else if(rr[i].type == 15)
            ptr += 3;
        memset(rr[i].rdata, 0, sizeof(rr[i].rdata));
        if(rr[i].type == 1)
        {
            char *ip;
            struct in_addr addr;
            unsigned long l;
            l = *((unsigned long*)ptr);
            memcpy(&addr, &l, 4);
            ip = inet_ntoa(addr);
            size_t ip_len = strlen(ip);
             if (ip_len < sizeof(rr[i].rdata)) {
                memcpy(rr[i].rdata, ip, ip_len);
            } else {
                printf("IP too long\n");
                memcpy(rr[i].rdata, ip, 128);
            }
        }

        else
        {
            memcpy(rr[i].rdata, ptr, rr[i].data_len);
        }
        ptr += rr[i].data_len;
    }
    return ptr;
}

void getAddRR(struct DNSRR* addrr, struct DNSHeader* header, unsigned char* ptr)
{
    ptr++;
    int i, flag, num;
    for(i = 0; i < header->addNum; i++)
    {
        num = 0;
        for(;;)
        {
            flag = (int)ptr[0];
            num += (flag + 1);
            ptr += (flag + 1);
            if(flag == 0)
                break;
        }
        ptr -= num;
        memset(addrr->name, 0, sizeof(addrr->name));
        memcpy(addrr->name, ptr, num - 1);
        ptr += num;
        addrr->type = ntohs(*((unsigned short*)ptr));
        ptr += 2;
        addrr->_class = ntohs(*((unsigned short*)ptr));
        ptr += 2;
        addrr->ttl = ntohl(*((unsigned short*)ptr));
        ptr += 4;
        addrr->data_len = ntohs(*((unsigned short*)ptr));
        ptr += 2;
        memset(addrr->rdata, 0, sizeof(addrr->rdata));
        char *ip;
        struct in_addr addr;
        unsigned long l;
        l = *((unsigned long*)ptr);
        memcpy(&addr, &l, 4);
        ip = inet_ntoa(addr);
        memcpy(addrr->rdata, ip, strlen(ip));
        ptr += addrr->data_len;
    }
}
