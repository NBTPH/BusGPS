#include <LC76G.h>

char* field_parse(char *out_buff, const size_t out_length, const char *start){ //start has to point at the start of an element, return is the pointer to the next comma
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

static void print_gga_data(const GGA_MSG_t *msg) {
    printf("================= NMEA GGA MESSAGE =================\n");

    // TIME
    printf(" UTC TIME  | %02d:%02d:%05.2f\n", 
            msg->UTC.Hours, msg->UTC.Minutes, msg->UTC.Seconds);

    // LATITUDE
    if (msg->Lat != -1.0f) {
        printf(" LATITUDE  | %10.6f %c\n", msg->Lat, msg->NS);
    } else {
        printf(" LATITUDE  | N/A\n");
    }

    // LONGITUDE
    if (msg->Lon != -1.0f) {
        printf(" LONGITUDE | %10.6f %c\n", msg->Lon, msg->EW);
    } else {
        printf(" LONGITUDE | N/A\n");
    }

    // FIX QUALITY
    printf(" QUALITY   | ");
    switch(msg->Quality) {
        case 0: printf("0 - Invalid/No Fix\n"); break;
        case 1: printf("1 - GPS Fix (SPS)\n"); break;
        case 2: printf("2 - DGPS Fix\n"); break;
        case 3: printf("3 - PPS Fix\n"); break;
        case 4: printf("4 - Real Time Kinematic (RTK)\n"); break;
        case 5: printf("5 - Float RTK\n"); break;
        case 6: printf("6 - Estimated (Dead Reckoning)\n"); break;
        default: printf("%d - Unknown\n", msg->Quality); break;
    }

    // SATELLITES & PRECISION
    printf(" SATELLITES| %d used\n", msg->NumSatUsed);
    printf(" HDOP      | %6.3f\n", msg->HDOP);

    // ALTITUDE & SEPARATION
    printf(" ALTITUDE  | %7.3f meters (MSL)\n", msg->Alt);
    printf(" GEOID SEP | %7.3f meters\n", msg->Sep);

    // CHECKSUM
    printf(" CHECKSUM  | %d\n", msg->CSM);
    printf("====================================================\n\n\n\r");
}

bool Parse_GGA_MSG(const char *const p_start, unsigned int Length, GGA_MSG_t *msg_data){
    if(p_start == NULL || msg_data == NULL){
        return false;
    }
    //$<TalkerID>GGA,<UTC>,<Lat>,<N/S>,<Lon>,<E/W>,<Quality>,<NumSatUsed>,<HDOP>,<Alt>,M,<Sep>,M,<DiffAge>,<DiffStation>*<Checksum><CR><LF>
    printf("ENTER PARSING\n");
    char parsed_buffer[20] = {0};
    char temp_buffer[10] = {0};

    //Start with validating CSM first
    const char *p_str = &p_start[Length - 3]; //pointer to the first element of checksum
    // printf("%c\n", (char)*p_str);
    strncpy(parsed_buffer, p_str, 2); //copy 2 elements from pointer
    msg_data->CSM = strtol(parsed_buffer, NULL, 16); //convert those 2 ascii hex element to a number

    uint8_t actual_CSM = Ql_Check_XOR(p_start, (p_str - 1) - p_start); //calculate actual checksum on string MSG
    // printf("Calculated CSM %d\n", actual_CSM);
    if(actual_CSM != msg_data->CSM){
        printf("Error[Parse_GGA_MSG]: CHECKSUM FAILED \n\r");
        return false;
    }

    p_str = p_start + 6; //move to the first element of UTC

    //UTC
    char *next_comma = field_parse(parsed_buffer, sizeof(parsed_buffer), p_str);

    strncpy(temp_buffer, &parsed_buffer[0], 2); //first two digit of UTC is hours
    msg_data->UTC.Hours = atoi(temp_buffer);

    strncpy(temp_buffer, &parsed_buffer[2], 2); //second two digit of UTC is minutes
    msg_data->UTC.Minutes = atoi(temp_buffer);

    strcpy(temp_buffer, &parsed_buffer[4]); //rest of buffer is floating point seconds number, longer than 2 characters will go into 0 filled field and terminate
    msg_data->UTC.Seconds = atof(temp_buffer);
    
    memset(parsed_buffer, 0, sizeof(parsed_buffer)); //reset both
    memset(temp_buffer, 0, sizeof(temp_buffer));

    //Lat
    p_str = next_comma + 1; //move to first digit of Lat
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
    p_str += ((char)*p_str == ',') ? 1 : 2; //if field is empty then move only 1 to get to the first element of the next field, othewise 2 because we need to skip a comma
    
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
    p_str += ((char)*p_str == ',') ? 1 : 2; //if field is empty then move only 1 to get to the first element of the next field, othewise 2 because we need to skip a comma

    //Quality
    msg_data->Quality = (char)*p_str - '0'; //this field never empty so we don't need to check
    p_str += 2; //Move to next field

    //NumSatUsed
    next_comma = field_parse(parsed_buffer, sizeof(parsed_buffer), p_str); 
    msg_data->NumSatUsed = (uint8_t)atoi(parsed_buffer); //this field never empty so we don't need to check
    memset(parsed_buffer, 0, sizeof(parsed_buffer)); //reset buffer
    p_str = next_comma + 1;//move to first element of next field

    //HDOP
    next_comma = field_parse(parsed_buffer, sizeof(parsed_buffer), p_str);
    if(next_comma == NULL){//if it returns NULL, that means this field is empty
        msg_data->HDOP = -1;
        p_str = p_str + 1; //move to Alt
    }
    else{
        msg_data->HDOP = atof(parsed_buffer);
        memset(parsed_buffer, 0, sizeof(parsed_buffer)); //reset buffer
        p_str = next_comma + 1;//move to Alt
    }

    //Alt
    next_comma = field_parse(parsed_buffer, sizeof(parsed_buffer), p_str);
    if(next_comma == NULL){//if it returns NULL, that means this field is empty
        msg_data->Alt = -1;
        p_str = p_str + 3; //move to Sep
    }
    else{
        msg_data->Alt = atof(parsed_buffer);
        memset(parsed_buffer, 0, sizeof(parsed_buffer)); //reset buffer
        p_str = next_comma + 3;//move to Sep
    }

    //Sep
    next_comma = field_parse(parsed_buffer, sizeof(parsed_buffer), p_str);
    if(next_comma == NULL){//if it returns NULL, that means this field is empty
        msg_data->Sep = -1;
    }
    else{
        msg_data->Sep = atof(parsed_buffer);
    }

    print_gga_data(msg_data);

    return true;
}

static void print_rmc_data(const RMC_MSG_t *msg) {
    printf("================= NMEA RMC MESSAGE =================\n");

    printf(" DATE/TIME | %02d/%02d/20%02d at %02d:%02d:%05.2f UTC\n", 
            msg->DATE.Day, msg->DATE.Month, msg->DATE.Year,
            msg->UTC.Hours, msg->UTC.Minutes, msg->UTC.Seconds);

    printf(" STATUS    | %s\n", (msg->Status == 'A') ? "VALID (A)" : "VOID (V)");

    if (msg->Lat != -1) {
        printf(" LATITUDE  | %10.6f %c\n", msg->Lat, msg->NS);
    } else {
        printf(" LATITUDE  | N/A\n");
    }

    if (msg->Lon != -1) {
        printf(" LONGITUDE | %10.6f %c\n", msg->Lon, msg->EW);
    } else {
        printf(" LONGITUDE | N/A\n");
    }

    printf(" SOG (SPD) | %6.3f knots\n", msg->SOG);
    printf(" COG (TRK) | %6.3f degrees\n", msg->COG);

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
    printf("====================================================\n\n\n\r");
}

bool Parse_RMC_MSG(const char *const p_start, unsigned int Length, RMC_MSG_t *msg_data){
    if(p_start == NULL || msg_data == NULL){
        return false;
    }
    // $<TalkerID>RMC,<UTC>,<Status>,<Lat>,<N/S>,<Lon>,<E/W>,<SOG>,<COG>,<Date>,<MagVar>,<MagVarDir>,<ModeInd>,<NavStatus>*<Checksum><CR><LF> 
    printf("ENTER PARSING\n");
    char parsed_buffer[20] = {0};
    char temp_buffer[10] = {0};

    //Start with validating CSM first
    const char *p_str = &p_start[Length - 3]; //pointer to the first element of checksum
    // printf("%c\n", (char)*p_str);
    strncpy(parsed_buffer, p_str, 2); //copy 2 elements from pointer
    msg_data->CSM = strtol(parsed_buffer, NULL, 16); //convert those 2 ascii hex element to a number

    uint8_t actual_CSM = Ql_Check_XOR(p_start, (p_str - 1) - p_start); //calculate actual checksum on string MSG
    // printf("Calculated CSM %d\n", actual_CSM);
    if(actual_CSM != msg_data->CSM){
        printf("Error[Parse_RMC_MSG]: CHECKSUM FAILED \n\r");
        return false;
    }

    p_str = p_start + 6; //move to the first element of UTC

    //UTC
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

    print_rmc_data(msg_data);

    return true;
}   