#include <stdio.h>
#include <stdlib.h>
#include "soapStub.h"
#include "reportTHInfo.nsmap"
//#include "reportTHInfo.h"

int reportTHInfo(const char *server, char *addr, float wd, float sd)
{
        struct soap SmsWBS_soap;
		struct ns1__thInfo arg0 = {addr, &sd, &wd};
		struct ns1__reportTHInfo infos;
		struct ns1__reportTHInfoResponse * result;
		int ret;

        infos.__sizearg0 = 1;
        infos.arg0 = &arg0;

        soap_init(&SmsWBS_soap);
        soap_set_namespaces(&SmsWBS_soap, namespaces);

        ret = soap_call___ns1__reportTHInfo(&SmsWBS_soap, server, "reportTHInfo", &infos, result);

        if(SmsWBS_soap.error)
        {
            printf("line[%d] soap error:%d, %s, %s \n", __LINE__, 
				SmsWBS_soap.error, *soap_faultcode(&SmsWBS_soap), *soap_faultstring(&SmsWBS_soap));
         }
		else
			printf("code = %d message = %s\n", result->return_->code, result->return_->message);

        soap_end(&SmsWBS_soap);
        soap_done(&SmsWBS_soap);

        return ret;
}

int main(int argc, char **argv)
{
        char* server=argv[1];

        float wd,sd;
		char message[250] = {0};
		int ret;

        if( argc < 5 )
        {
            printf("usage: %s addr wd sd \n", argv[0]);
            exit(0);
        }
        wd = atof(argv[3]);
		wd = ((int)(wd * 10)) / 10.0;
        sd = atof(argv[4]);
		sd = ((int)(sd * 100)) / 100.0;
        ret = reportTHInfo(server, argv[2], wd, sd);

        return 0;
}
