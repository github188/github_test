//gsoap ns1  service name:	reportTHInfo 
//gsoap ns1  service type:	IEnvironmentWebService 
//gsoap ns1  service port:	http://172.31.114.133:8888/EW_WS/EnvironmentService 
//gsoap ns1  service namespace:	http://webservice.ewws.fri.com/ 
//gsoap ns1  service transport:	http://schemas.xmlsoap.org/soap/http 
struct ns1__thInfo
{
/// Element "ljdz" of XSD type xs:string.
    char*                                ljdz                           0;	///< Optional element.
/// Element "sd" of XSD type xs:float.
    float*                               sd                             0;	///< Optional element.
/// Element "wd" of XSD type xs:float.
    float*                               wd                             0;	///< Optional element.
};

struct ns1__reportTHInfo
{
/// Size of array of struct ns1__thInfo* is 0..unbounded.
   $int                                  __sizearg0                     0;
/// Array struct ns1__thInfo* of size 0..unbounded.
    struct ns1__thInfo*                  arg0                           0;
};

struct ns1__resultInfo
{
/// Element "code" of XSD type xs:int.
    int                                  code                           1;	///< Required element.
/// Element "message" of XSD type xs:string.
    char*                                message                        0;	///< Optional element.
};

struct ns1__reportTHInfoResponse
{
/// Element "return" of XSD type "http://webservice.ewws.fri.com/":resultInfo.
    struct ns1__resultInfo*              return_                        0;	///< Optional element.
};

/*
typedef struct
{
    char *ljdz;
	float wd;
	float sd;
}ns__thInfo;

typedef struct
{
    int code	1;
	char * message	0;
}ns__resultInfo;*/

//gsoap ns1  service method-protocol:	reportTHInfo SOAP
//gsoap ns1  service method-style:	reportTHInfo document
//gsoap ns1  service method-encoding:	reportTHInfo literal
//gsoap ns1  service method-action:	reportTHInfo ""
//gsoap ns1  service method-output-action:	reportTHInfo Response
int __ns1__reportTHInfo(
    struct ns1__reportTHInfo*           ns1__reportTHInfo,	///< Input parameter
    struct ns1__reportTHInfoResponse   *ns1__reportTHInfoResponse	///< Output parameter
);