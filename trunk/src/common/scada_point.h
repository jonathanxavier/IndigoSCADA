#if defined( _MSC_VER)          /* Microsoft C */
#pragma pack(1)             /* Byte Alignment   */
#endif

struct scada_point {
	char name[35];
	char tag[20];
	//double previous_value;
	double current_value;
	double next_value;
	unsigned char write_to_driver;
	unsigned char checksum;
};

#if defined( _MSC_VER)          /* Microsoft C */
#pragma pack()              /* Byte Alignment   */
#endif