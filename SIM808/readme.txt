GSM.h--->sim808.h
GSM3ShieldV1BaseProvider.h--->sim808BaseProvider.h
GSM3MobileAccessProvider.h--->sim808mobileaccessprovider.h
GSM3ShieldV1ModemCore.h--->sim808ModemCore.h
GSM3CircularBuffer--->sim808CircularBuffer.h
GSM3SoftSerial.h--->sim808Timer2Serial.h
GSM3ShieldV1AccessProvider.h--->sim808AccessProvider.h
GSM3MobileVoiceProvider.h--->SIM808MobileVoiceProvider.h
GSM3MobileSMSProvider--->sim808MobileVoiceProvider.h
GSM3SMSService.h--->sim808SMSService.h	
GSM3ShieldV1SMSProvider--->sim808SMSProvider.h	


注：PORTB 表示存储在flash中的函数内变量
    PROGMEM 表示存储在flash中的全局变量 

	SIM808ModemCore_t是一个公用的对象，其中class中有SIM808Timer2Serial类型的成员