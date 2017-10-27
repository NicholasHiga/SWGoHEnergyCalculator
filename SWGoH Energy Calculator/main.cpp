#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <ctime>
#include <regex>

using namespace std;
using std::chrono::system_clock;

const std::string FILENAME = "ecalc_settings.txt";
const int REFRESH_AMOUNT = 45;
const int REFRESH_PER_ENERGY = 6;  // Minutes
const int REFRESH_PER_CANTINA_ENERGY = 12; // Minutes

tm times[6];
int maxEnergies[2];

fstream settings;
system_clock::time_point currentTime;

enum TIMES
{
	DAY_RESET,
	GUILD_RESET,
	ENERGY1,
	ENERGY2,
	ENERGY3,
	CANTINA,
	TIMES_SIZE
};

enum ENERGY_TYPE
{
	NORMAL_E,
	CANTINA_E,
	ETYPE_SIZE
};

const string TIME_STRINGS[] =
{
	"DayResetTime", "day reset",
	"GuildResetTime", "guild reset",
	"FirstEnergyRefresh", "first energy refresh",
	"SecondEnergyRefresh", "second energy refresh",
	"ThirdEnergyRefresh", "third energy refresh",
	"CantinaEnergyRefresh", "cantina energy refresh",
};

const string MAX_ENERGY_STRINGS[] =
{
	"MaxEnergy", "max energy",
	"MaxCantinaEnergy", "max cantina energy"
};

bool
doSettingsExist()
{
	settings.open(FILENAME);
	if (!settings.is_open())
		return false;
	settings.close();
	return true;
}

bool
doArgumentsValidate(int argc, char ** argv)
{
	return false;
}

string
trimWhitespace(string value)
{
	return regex_replace(value, regex("^ +| +$|( ) +"), "$1");
}


// Check if time is in proper HH:MM format. 
bool
isValidTime(string time)
{
	// Valid format examples: 24 hour clock
	// 0:30, 00:30, 5:00, 5, 5:30, 13:30

	// Invalid examples
	// 1 PM, 24:00, 25:20
	// 7: 30, 7:30PM, 7:90

	// Trimming excess whitespace
	time = trimWhitespace(time);

	regex hoursMinutesFormat("([0-9]|0[0-9]|1[0-9]|2[0-3]):[0-5][0-9]");
	if (!regex_match(time, hoursMinutesFormat))
	{
		cout << time << " is invalid. " << endl;
		return false;
	}
	return true;
}

// Sentence format: 2 hours 50 minutes
//             Not: 02:50
// Does not check the inputs for correct values.
string
timeToString(tm time, bool sentenceFormat)
{
	if (!sentenceFormat)
	{
		stringstream strTime;
		strTime << setfill('0') << setw(2) << to_string(time.tm_hour) << ":"
			<< setfill('0') << setw(2) << to_string(time.tm_min);
		return strTime.str();
	}
	else
	{
		string strTime;
		if (time.tm_hour != 0)
		{
			strTime = to_string(time.tm_hour);
			if (time.tm_hour == 1)
				strTime += " hour";
			else
				strTime += " hours";
			
			if (time.tm_min != 0)
				strTime += " ";
		}

		if (time.tm_min == 1)
			strTime += to_string(time.tm_min) + " minute";
		else if ((time.tm_min != 0 && time.tm_hour != 0) || time.tm_hour == 0)
			strTime += to_string(time.tm_min) + " minutes";

		return strTime;
	}
}

tm
stringToTime(string time, bool *validTime = nullptr)
{
	tm tmObject;
	if (isValidTime(time))
	{
		int colonPos = time.find_first_of(":");
		tmObject.tm_hour = stoi(time.substr(0, colonPos));
		tmObject.tm_min = stoi(time.substr(colonPos + 1));

		if (validTime != nullptr)
			*validTime = true;
	}
	else
	{
		if (validTime != nullptr)
			*validTime = false;
	}

	return tmObject;
}

void
setTimeSettings(tm time, TIMES whatTime)
{
	times[whatTime] = time;
}

void
setMaxEnergy(int value, ENERGY_TYPE eType)
{
	maxEnergies[eType] = value;
}

bool
readSettings()
{
	// TODO: Check for corrupt settings.
	if (doSettingsExist())
	{
		return true;
	}
	else
		return false;
}

void
writeSettings()
{
	settings.open(FILENAME, fstream::out);

	for (size_t i = 0; i < TIMES::TIMES_SIZE; ++i)
	{
		settings << TIME_STRINGS[i * 2] << setfill('0') << " "
			<< setw(2) << to_string(times[i].tm_hour) << ":"
			<< setw(2) << to_string(times[i].tm_min) << endl;
	}

	for (size_t i = 0; i < ENERGY_TYPE::ETYPE_SIZE; ++i)
	{
		settings << MAX_ENERGY_STRINGS[i * 2] << " "
			<< to_string(maxEnergies[i]) << endl;
	}

	settings.close();
}

void
showPrompts()
{
	if (!readSettings())
	{
		cout << "This is your first time running ecalc, you will be";
		cout << " asked a few questions which you only need to answer";
		cout << " one time." << endl;
	}
}

void
firstTimeSetup()
{
	string input;
	for (size_t i = 0; i < TIMES::TIMES_SIZE; ++i)
	{
		do
		{
			cout << "When is your " << TIME_STRINGS[i * 2 + 1]
				<< " time in the 24-hour format HH:MM? ";
			cin >> input;
		} 
		while (!isValidTime(input));
		setTimeSettings(stringToTime(input), static_cast<TIMES>(i));
	}

	bool inputValid;
	regex energyFormat("[0-9]{2,3}");
	for (size_t i = 0; i < ENERGY_TYPE::ETYPE_SIZE; ++i)
	{
		int energy;
		do
		{
			cout << "What is your " << MAX_ENERGY_STRINGS[i * 2 + 1] << "? ";
			cin >> input;

			inputValid = regex_match(input, energyFormat);
			if (!inputValid)
				cout << input << " is invalid. " << endl;
			else
				energy = stoi(input);
		}
		while (!inputValid);
		setMaxEnergy(energy, static_cast<ENERGY_TYPE>(i));
	}

	cout << "First time setup complete." << endl;
}

int
numberOfMinutesInBetween(tm firstTime, tm secondTime)
{
	// Test cases
	// 00:00, 23:59 - 23 hours, 59 mins
	// 00:50, 12:30 - 11 hours, 40 mins
	// 00:50, 23:30 - 22 hours, 40 mins
	// 12:30, 00:50 - 12 hours, 20 mins
	// 23:30, 00:50 -  1 hour,  20 mins
	// 12:50, 00:30	- 11 hours, 40 mins
	// 23:50, 00:30 -  0 hours, 40 mins

	int firstTotalMinutes, secondTotalMinutes;
	int numMinutesInBetween;

	firstTotalMinutes = firstTime.tm_hour * 60 + firstTime.tm_min;
	secondTotalMinutes = secondTime.tm_hour * 60 + secondTime.tm_min;
	numMinutesInBetween = abs(firstTotalMinutes - secondTotalMinutes);

	if (secondTotalMinutes > firstTotalMinutes)
		return numMinutesInBetween;
	else
		return (24 * 60) - numMinutesInBetween;
}

// Make it so at the given guild reset time, you will have max energy if you
// make it so your current energy is the returned value.
int
calcCurrentEnergyForGuildReset(int maxEnergy, int timePerSingleEnergy, 
	tm guildResetTime, const tm *customCurrentTime = nullptr)
{
	// TODO: Use custom current time.
	time_t t = time(0);
	tm *currentTime = localtime(&t);

	int totalMinutes = numberOfMinutesInBetween(*currentTime, guildResetTime);
	return max(0, maxEnergy - totalMinutes / timePerSingleEnergy);
}

// Returns in hours, minutes
tm 
timeToGetFullEnergy(int currentEnergy, int maxEnergy, int timePerSingleEnergy,
	tm energyResetTimes[6], const tm *customCurrentTime = nullptr)
{
	// TODO: Account for energy reset times, use custom current time.
	tm time;
	time.tm_hour = 0;
	time.tm_min = 0;
	if (currentEnergy > maxEnergy)
		return time;

	int neededEnergy = maxEnergy - currentEnergy;
	int minsToMax = neededEnergy * timePerSingleEnergy;

	time.tm_hour = minsToMax / 60;
	time.tm_min = minsToMax % 60;

	return time;
}

tm 
addTime(tm originalTime, tm additionalTime)
{
	originalTime.tm_hour = (originalTime.tm_hour + additionalTime.tm_hour) 
		% 24 + (originalTime.tm_min + additionalTime.tm_min) / 60;
	originalTime.tm_min = (originalTime.tm_min + additionalTime.tm_min) % 60;
	return originalTime;
}

tm
whatTimeIsFullEnergy(int currentEnergy, int maxEnergy, int timePerSingleEnergy,
	tm energyResetTimes[6], const tm *customCurrentTime = nullptr)
{
	// TODO: Account for energy reset times and customCurrentTime
	tm timeFromNowForFullEnergy = timeToGetFullEnergy(currentEnergy, maxEnergy,
		timePerSingleEnergy, energyResetTimes, customCurrentTime);

	time_t t = time(0);
	tm *fullEnergyTime = localtime(&t);
	return addTime(*fullEnergyTime, timeFromNowForFullEnergy);
}

int
main(int argc, char ** argv)
{
	int i;
	for (i = 0; i < argc; i++) {
		printf("Argument %i = %s\n", i, argv[i]);
	}

	/*tm test = { 0 };
	test.tm_hour = 13;
	test.tm_min = 30;
	cout << calcCurrentEnergyForGuildReset(144, 6, test);*/

	//cout << timeToString(timeToGetFullEnergy(143, 144, 6, times), true)
	//	<< endl;
	//cout << timeToString(whatTimeIsFullEnergy(0, 144, 6, times), false) 
	//	<< endl;

	firstTimeSetup();
	writeSettings();

	if (argc == 1)
	{
		//showPrompts()
	}
	else
	{

	}

	int x;
	cin >> x;

	return 0;
}