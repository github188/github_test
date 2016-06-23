#include "soapH.h"
#include "reportTHInfo.nsmap"
#include <stdio.h>
#include <stdlib.h>
//#include "reportTHInfo.h"

int main(int argc, char **argv)
{
        int m, s;               /* master and slave sockets */
        struct soap SmsWBS_soap;

        soap_init(&SmsWBS_soap);

        soap_set_namespaces(&SmsWBS_soap, namespaces);

        if (argc < 2)
        {
                printf("usage: %s <server_port> \n", argv[0]);
                exit(1);
        }
        else
        {
                m = soap_bind(&SmsWBS_soap, NULL, atoi(argv[1]), 100);
                if (m < 0)
                {
                        soap_print_fault(&SmsWBS_soap, stderr);
                        exit(-1);
                }

                fprintf(stderr, "Socket connection successful: master socket = %d\n", m);

                for (;;)
                {
                        s = soap_accept(&SmsWBS_soap);

                        if (s < 0)
                        {
                                soap_print_fault(&SmsWBS_soap, stderr);
                                exit(-1);
                        }

                        fprintf(stderr, "Socket connection successful: slave socket = %d\n", s);
                        soap_serve(&SmsWBS_soap);
                        soap_end(&SmsWBS_soap);
                }

       }

        return 0;
}

int ns__reportTHInfo(struct soap *add_soap, thInfo infos[], resultInfo *result)
{
        printf("addr=%s,wd=%f, sd=%f\n", infos[0].ljdz, infos[0].wd, infos[0].sd);
		result->code = 24;
		strcpy(result->message, "OK");
        return 0;
}
