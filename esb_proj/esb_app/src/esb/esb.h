#ifndef ESB_H
#define ESB_H

typedef struct {
    const unsigned char * key;
    void * value;
}
pair;

typedef struct {
    const unsigned char * MessageID;
    const unsigned char * MessageType;
    const unsigned char * Sender;
    const unsigned char * Destination;
    const unsigned char * CreationDateTime;
    const unsigned char * Signature;
    pair * UserProperties[50];
    const unsigned char * ReferenceID;

}
envelope;

typedef struct {
    const unsigned char * data;
}
payload;

typedef struct {
    envelope * bmd_envelope;
    payload * bmd_payload;
}
BMD;

typedef struct {
     int val;
     char * Transform_key;
     char * Transform_value;
     char * Transport_key;
     char * Transport_value;
}
TD; 

TD  process_esb_request(char * bmd_file_path);
TD  is_bmd_valid(BMD * bmd);
BMD * parse_bmd_xml(char * );
BMD * process_xml(char * );
int check_if_string_is_guid(const unsigned char * value);
envelope * extract_envelop(char * bmd_xml);
payload * extract_payload(char * bmd_xml);


#define INSERT_IN_ESB_REQUEST "INSERT INTO esb_request(sender_id,dest_id,message_type,reference_id,message_id,received_on,data_location,status) VALUES ('%s','%s','%s','%s','%s','%s','%s','%s');"
#define CHECK_TRANSFORM "SELECT config_key, config_value FROM transform_config WHERE route_id = '%s';"
#define CHECK_TRANSPORT "SELECT config_key, config_value FROM transport_config WHERE route_id = '%s';"

#endif