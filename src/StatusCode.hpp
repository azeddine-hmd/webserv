#ifndef STATUS_CODE_HPP
#define STATUS_CODE_HPP

#include <string>

namespace HttpStatus
{
	enum statusCode
	{
			none = 0,
		/* 1xx: Informational 
			It means the request has been received and the 
			process is continuing.
		*/
			continueCode= 100,
			switchingProtocols= 101,

		/* 2xx: Success 
			It means the action was successfully received,
			understood, and accepted.
		*/
			ok= 200,
			created= 201,
			accepted= 202,
			nonAuthoritativeInformation= 203,
			noContent= 204,
			resetContent= 205,

		/* 3xx: Redirection 
			It means further action must be taken in order to
			complete the request.
		*/
			multipleChoices= 300,
			movedPermanently= 301,
			found= 302,
			seeOther= 303,
			useProxy= 305,
			temporaryRedirect= 307,

		/* 4xx: Client Error 
			It means the request contains incorrect syntax or 
			cannot be fulfilled.
		*/
			badRequest= 400,
			paymentRequired= 402,
			forbidden= 403,
			notFound= 404,
			methodNotAllowed= 405,
			notAcceptable= 406,
			requestTimeout= 408,
			conflict= 409,
			gone= 410,
			lengthRequired= 411,
			payloadTooLarge= 413,
			URITooLong= 414,
			unsupportedMediaType= 415,
			expectationFailed= 417,
			upgradeRequired= 426,

		/* 5xx: Server Error 
			It means the server failed to fulfill an apparently valid
			request.
		*/
			internalServerError= 500,
			notImplemented= 501,
			badGateway= 502,
			serviceUnavailable= 503,
			gatewayTimeout= 504,
			httpVersionNotSupported= 505,
			variantAlsoNegotiates         = 506,
			insufficientStorage           = 507,
			loopDetected                  = 508,
			notExtended                   = 510,
			networkAuthenticationRequired = 511
			
	};

	inline const char * reasonPhrase(statusCode code)
	{
		switch ((int)code)
		{
			case 0: return "None";
			case 100: return "Continue";
			case 101: return "Switching Protocols";
			case 102: return "Processing";
			case 103: return "Early Hints";
			case 200: return "OK";
			case 201: return "Created";
			case 202: return "Accepted";
			case 203: return "Non-Authoritative Information";
			case 204: return "No Content";
			case 205: return "Reset Content";
			case 206: return "Partial Content";
			case 207: return "Multi-Status";
			case 208: return "Already Reported";
			case 226: return "IM Used";
			case 300: return "Multiple Choices";
			case 301: return "Moved Permanently";
			case 302: return "Found";
			case 303: return "See Other";
			case 304: return "Not Modified";
			case 305: return "Use Proxy";
			case 307: return "Temporary Redirect";
			case 308: return "Permanent Redirect";
			case 400: return "Bad Request";
			case 401: return "Unauthorized";
			case 402: return "Payment Required";
			case 403: return "Forbidden";
			case 404: return "Not Found";
			case 405: return "Method Not Allowed";
			case 406: return "Not Acceptable";
			case 407: return "Proxy Authentication Required";
			case 408: return "Request Timeout";
			case 409: return "Conflict";
			case 410: return "Gone";
			case 411: return "Length Required";
			case 412: return "Precondition Failed";
			case 413: return "Payload Too Large";
			case 414: return "URI Too Long";
			case 415: return "Unsupported Media Type";
			case 416: return "Range Not Satisfiable";
			case 417: return "Expectation Failed";
			case 418: return "I'm a teapot";
			case 422: return "Unprocessable Entity";
			case 423: return "Locked";
			case 424: return "Failed Dependency";
			case 426: return "Upgrade Required";
			case 428: return "Precondition Required";
			case 429: return "Too Many Requests";
			case 431: return "Request Header Fields Too Large";
			case 451: return "Unavailable For Legal Reasons";
			case 500: return "Internal Server Error";
			case 501: return "Not Implemented";
			case 502: return "Bad Gateway";
			case 503: return "Service Unavailable";
			case 504: return "Gateway Time-out";
			case 505: return "HTTP Version Not Supported";
			case 506: return "Variant Also Negotiates";
			case 507: return "Insufficient Storage";
			case 508: return "Loop Detected";
			case 510: return "Not Extended";
			case 511: return "Network Authentication Required";
			default: return NULL;
		}
	}

}

#endif