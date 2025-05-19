////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Defines a precise 64-bits time structure measuring time in milliseconds and handling overflow after 50 days
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

// Time structure, storing the time in milliseconds on 64-bits
struct Time_ms {
	U64	time_ms = 0;

	Time_ms() { GetTime(); }

	void	GetTime();

	float	GetTime_seconds() {
		return 0.001f * time_ms;
	}
};

struct DateTime {
	U16		year;
	U16		day;	// Day of the year in [0,364]

	Time_ms	time;

	DateTime() {
		year = 0;
		day = 0;
	}
};
