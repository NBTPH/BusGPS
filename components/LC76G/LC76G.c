#include <LC76G.h>

char* field_parse(char *out_buff, const size_t out_length, const char *start){ //start has to point at the start of the element, return is the pointer to the next comma
    memset(out_buff, 0, sizeof(char) * out_length);
    if((char)*start == ','){//if start index is another comma
        return NULL;
    }
    char c_comma = ',';
    char *next_comma = strchr(start, c_comma); //find next comma
    if(next_comma != NULL){
        size_t length = next_comma - start;
        if(length <= out_length){
            strncpy(out_buff, start, length);
            out_buff[length] = '\0';
            return next_comma;
        }
    }
    return NULL;
}

uint8_t Ql_Check_XOR(const char *pData, unsigned int Length){ 
    unsigned char result = 0; 
    unsigned int i = 0; 
 
    if((NULL == pData) || (Length < 1)) 
    { 
        return 0; 
    } 
    for(i = 0; i < Length; i++) 
    { 
        result ^= *(pData + i); 
    } 
 
    return result; 
}

static void print_rmc_data(const RMC_MSG_t *msg) {
    printf("================= NMEA RMC MESSAGE =================\n");

    printf(" DATE/TIME | %02d/%02d/20%02d at %02d:%02d:%05.2f UTC\n", 
            msg->DATE.Day, msg->DATE.Month, msg->DATE.Year,
            msg->UTC.Hours, msg->UTC.Minutes, msg->UTC.Seconds);

    printf(" STATUS    | %s\n", (msg->Status == 'A') ? "VALID (A)" : "VOID (V)");

    if (msg->Lat != -1) {
        printf(" LATITUDE  | %10.4f %c\n", msg->Lat, msg->NS);
    } else {
        printf(" LATITUDE  | N/A\n");
    }

    if (msg->Lon != -1) {
        printf(" LONGITUDE | %10.4f %c\n", msg->Lon, msg->EW);
    } else {
        printf(" LONGITUDE | N/A\n");
    }

    printf(" SOG (SPD) | %6.2f knots\n", msg->SOG);
    printf(" COG (TRK) | %6.2f degrees\n", msg->COG);

    printf(" MODE      | ");
    switch(msg->ModeInd) {
        case 'A': printf("Autonomous\n"); break;
        case 'D': printf("Differential\n"); break;
        case 'E': printf("Estimated\n"); break;
        case 'M': printf("Manual\n"); break;
        case 'N': printf("Data Not Valid\n"); break;
        default:  printf("Unknown (%c)\n", msg->ModeInd); break;
    }

    printf(" NAV STAT  | %c\n", (msg->NavStatus != '\0') ? msg->NavStatus : '-');
    printf(" CHECKSUM  | %d\n", msg->CSM);
    printf("====================================================\n");
}

bool Parse_GGA_MSG(const char *const p_start, unsigned int Length, GGA_MSG_t *msg_data){
    if(p_start == NULL || msg_data == NULL){
        return false;
    }
    // $<TalkerID>RMC,<UTC>,<Status>,<Lat>,<N/S>,<Lon>,<E/W>,<SOG>,<COG>,<Date>,<MagVar>,<MagVarDir>,<ModeInd>,<NavStatus>*<Checksum><CR><LF> 
    const char *p_str = p_start + 6;
    printf(" \n\n\n ENTER PARSING \n");
    //UTC
    char parsed_buffer[20] = {0};
    char temp_buffer[10] = {0};
    char *next_comma = field_parse(parsed_buffer, sizeof(parsed_buffer), p_str);

    strncpy(temp_buffer, &parsed_buffer[0], 2); //first two digit of UTC is hours
    msg_data->UTC.Hours = atoi(temp_buffer);

    strncpy(temp_buffer, &parsed_buffer[2], 2); //second two digit of UTC is minutes
    msg_data->UTC.Minutes = atoi(temp_buffer);

    strcpy(temp_buffer, &parsed_buffer[4]); //rest of buffer is floating point seconds number, longer than 2 characters will go into 0 filled field and terminate
    msg_data->UTC.Seconds = atof(temp_buffer);
    
    memset(parsed_buffer, 0, sizeof(parsed_buffer)); //reset both
    memset(temp_buffer, 0, sizeof(temp_buffer));

    //Status
    msg_data->Status = next_comma[1]; //status is just a character

    //Lat
    p_str = next_comma + 3; //move to first digit of Lat
    next_comma = field_parse(parsed_buffer, sizeof(parsed_buffer), p_str);
    if(next_comma == NULL){//if it returns NULL, that means this field is empty
        msg_data->Lat = -1;
        p_str = p_str + 1; //move to N/S char
    }
    else{
        msg_data->Lat = atof(parsed_buffer);
        memset(parsed_buffer, 0, sizeof(parsed_buffer)); //reset buffer
        p_str = next_comma + 1;//move to N/S char
    }

    //N/S
    msg_data->NS = ((char)*p_str == ',') ? '\0' : (char)*p_str; //if at N/S char field is another comma, that means this field is empty
    p_str += ((char)*p_str == ',') ? 1 : 2; //if field is empty then move only 2 to get to the first element of the next field, othewise 2 because we need to skip a comma
    
    //Lon
    next_comma = field_parse(parsed_buffer, sizeof(parsed_buffer), p_str);
    if(next_comma == NULL){//if it returns NULL, that means this field is empty
        msg_data->Lon = -1;
        p_str = p_str + 1; //move to N/S char
    }
    else{
        msg_data->Lon = atof(parsed_buffer);
        memset(parsed_buffer, 0, sizeof(parsed_buffer)); //reset buffer
        p_str = next_comma + 1;//move to N/S char
    }

    
    //E/W
    msg_data->EW = ((char)*p_str == ',') ? '\0' : (char)*p_str; //if at N/S char field is another comma, that means this field is empty
    p_str += ((char)*p_str == ',') ? 1 : 2; //if field is empty then move only 2 to get to the first element of the next field, othewise 2 because we need to skip a comma

    //SOG
    next_comma = field_parse(parsed_buffer, sizeof(parsed_buffer), p_str);
    if(next_comma == NULL){//if it moves to the next comma, that means this field is empty
        msg_data->SOG = -1;
        p_str = p_str + 1; //move to first element of next field
    }
    else{
        msg_data->SOG = atof(parsed_buffer);
        memset(parsed_buffer, 0, sizeof(parsed_buffer)); //reset buffer
        p_str = next_comma + 1;//move to first element of next field
    }


    //COG
    next_comma = field_parse(parsed_buffer, sizeof(parsed_buffer), p_str);
    if(next_comma == NULL){//if it moves to the next comma, that means this field is empty
        msg_data->COG = -1;
        p_str = p_str + 1; //move to first element of next field
    }
    else{
        msg_data->COG = atof(parsed_buffer);
        memset(parsed_buffer, 0, sizeof(parsed_buffer)); //reset buffer
        p_str = next_comma + 1;//move to first element of next field
    }

    //DATE
    next_comma = field_parse(parsed_buffer, sizeof(parsed_buffer), p_str);

    strncpy(temp_buffer, &parsed_buffer[0], 2); //first two digit of DATE is day
    msg_data->DATE.Day= atoi(temp_buffer);

    strncpy(temp_buffer, &parsed_buffer[2], 2); //second two digit of DATE is month
    msg_data->DATE.Month = atoi(temp_buffer);

    strncpy(temp_buffer, &parsed_buffer[4], 2); //last two digit of DATE is year
    msg_data->DATE.Year = atoi(temp_buffer);

    memset(parsed_buffer, 0, sizeof(parsed_buffer)); //reset both
    memset(temp_buffer, 0, sizeof(temp_buffer));

    //skip MagVar and MagVarDir because not supported
    p_str = next_comma + 3;
    // p_str = strchr(p_str + 1, c_comma);
    // p_str = p_str + 1;

    //ModeInd
    msg_data->ModeInd = (char)*p_str; //this field never empty so don't need to check for anything
    p_str = p_str + 2; //moves into NavStatus field

    //NavStatus
    msg_data->NavStatus = (char)*p_str;
    p_str = p_str + 2; //moves into Checksum field
    
    //Checksum
    strncpy(parsed_buffer, p_str, 2);
    msg_data->CSM = strtol(parsed_buffer, NULL, 16);

    printf("%.*s", Length, p_start);
    uint8_t actual_CSM = Ql_Check_XOR(p_start, (p_str - 1) - p_start);
    printf("Actual CSM %d\n", actual_CSM);
    print_rmc_data(msg_data);

    return true;
}

bool Parse_RMC_MSG(const char *const p_start, unsigned int Length, RMC_MSG_t *msg_data){
    if(p_start == NULL || msg_data == NULL){
        return false;
    }
    // $<TalkerID>RMC,<UTC>,<Status>,<Lat>,<N/S>,<Lon>,<E/W>,<SOG>,<COG>,<Date>,<MagVar>,<MagVarDir>,<ModeInd>,<NavStatus>*<Checksum><CR><LF> 
    const char *p_str = p_start + 6;
    printf(" \n\n\n ENTER PARSING \n");
    //UTC
    char parsed_buffer[20] = {0};
    char temp_buffer[10] = {0};
    char *next_comma = field_parse(parsed_buffer, sizeof(parsed_buffer), p_str);

    strncpy(temp_buffer, &parsed_buffer[0], 2); //first two digit of UTC is hours
    msg_data->UTC.Hours = atoi(temp_buffer);

    strncpy(temp_buffer, &parsed_buffer[2], 2); //second two digit of UTC is minutes
    msg_data->UTC.Minutes = atoi(temp_buffer);

    strcpy(temp_buffer, &parsed_buffer[4]); //rest of buffer is floating point seconds number, longer than 2 characters will go into 0 filled field and terminate
    msg_data->UTC.Seconds = atof(temp_buffer);
    
    memset(parsed_buffer, 0, sizeof(parsed_buffer)); //reset both
    memset(temp_buffer, 0, sizeof(temp_buffer));

    //Status
    msg_data->Status = next_comma[1]; //status is just a character

    //Lat
    p_str = next_comma + 3; //move to first digit of Lat
    next_comma = field_parse(parsed_buffer, sizeof(parsed_buffer), p_str);
    if(next_comma == NULL){//if it returns NULL, that means this field is empty
        msg_data->Lat = -1;
        p_str = p_str + 1; //move to N/S char
    }
    else{
        msg_data->Lat = atof(parsed_buffer);
        memset(parsed_buffer, 0, sizeof(parsed_buffer)); //reset buffer
        p_str = next_comma + 1;//move to N/S char
    }

    //N/S
    msg_data->NS = ((char)*p_str == ',') ? '\0' : (char)*p_str; //if at N/S char field is another comma, that means this field is empty
    p_str += ((char)*p_str == ',') ? 1 : 2; //if field is empty then move only 2 to get to the first element of the next field, othewise 2 because we need to skip a comma
    
    //Lon
    next_comma = field_parse(parsed_buffer, sizeof(parsed_buffer), p_str);
    if(next_comma == NULL){//if it returns NULL, that means this field is empty
        msg_data->Lon = -1;
        p_str = p_str + 1; //move to N/S char
    }
    else{
        msg_data->Lon = atof(parsed_buffer);
        memset(parsed_buffer, 0, sizeof(parsed_buffer)); //reset buffer
        p_str = next_comma + 1;//move to N/S char
    }

    
    //E/W
    msg_data->EW = ((char)*p_str == ',') ? '\0' : (char)*p_str; //if at N/S char field is another comma, that means this field is empty
    p_str += ((char)*p_str == ',') ? 1 : 2; //if field is empty then move only 2 to get to the first element of the next field, othewise 2 because we need to skip a comma

    //SOG
    next_comma = field_parse(parsed_buffer, sizeof(parsed_buffer), p_str);
    if(next_comma == NULL){//if it moves to the next comma, that means this field is empty
        msg_data->SOG = -1;
        p_str = p_str + 1; //move to first element of next field
    }
    else{
        msg_data->SOG = atof(parsed_buffer);
        memset(parsed_buffer, 0, sizeof(parsed_buffer)); //reset buffer
        p_str = next_comma + 1;//move to first element of next field
    }


    //COG
    next_comma = field_parse(parsed_buffer, sizeof(parsed_buffer), p_str);
    if(next_comma == NULL){//if it moves to the next comma, that means this field is empty
        msg_data->COG = -1;
        p_str = p_str + 1; //move to first element of next field
    }
    else{
        msg_data->COG = atof(parsed_buffer);
        memset(parsed_buffer, 0, sizeof(parsed_buffer)); //reset buffer
        p_str = next_comma + 1;//move to first element of next field
    }

    //DATE
    next_comma = field_parse(parsed_buffer, sizeof(parsed_buffer), p_str);

    strncpy(temp_buffer, &parsed_buffer[0], 2); //first two digit of DATE is day
    msg_data->DATE.Day= atoi(temp_buffer);

    strncpy(temp_buffer, &parsed_buffer[2], 2); //second two digit of DATE is month
    msg_data->DATE.Month = atoi(temp_buffer);

    strncpy(temp_buffer, &parsed_buffer[4], 2); //last two digit of DATE is year
    msg_data->DATE.Year = atoi(temp_buffer);

    memset(parsed_buffer, 0, sizeof(parsed_buffer)); //reset both
    memset(temp_buffer, 0, sizeof(temp_buffer));

    //skip MagVar and MagVarDir because not supported
    p_str = next_comma + 3;
    // p_str = strchr(p_str + 1, c_comma);
    // p_str = p_str + 1;

    //ModeInd
    msg_data->ModeInd = (char)*p_str; //this field never empty so don't need to check for anything
    p_str = p_str + 2; //moves into NavStatus field

    //NavStatus
    msg_data->NavStatus = (char)*p_str;
    p_str = p_str + 2; //moves into Checksum field
    
    //Checksum
    strncpy(parsed_buffer, p_str, 2);
    msg_data->CSM = strtol(parsed_buffer, NULL, 16);

    printf("%.*s", Length, p_start);
    uint8_t actual_CSM = Ql_Check_XOR(p_start, (p_str - 1) - p_start);
    printf("Actual CSM %d\n", actual_CSM);
    print_rmc_data(msg_data);

    return true;
}   