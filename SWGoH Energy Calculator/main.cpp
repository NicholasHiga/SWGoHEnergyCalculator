#include <iostream>
#include <iomanip>
#include <fstream>
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

tm times[5];
int maxEnergies[2];

fstream settings;
system_clock::time_point currentTime;

enum TIMES
{
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

// Check if time is in proper HH:MM format. If hours and minutes pointers
// are provided, store the values in said pointers.
bool
isValidTime(string time, int* hours = nullptr, int* minutes = nullptr)
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

	// Split the time itself with the period (AM / PM)
	int colonPos = time.find_first_of(":");

	if (hours)
		*hours = stoi(time.substr(0, colonPos));

	if (minutes)
		*minutes = stoi(time.substr(colonPos + 1));

	/*if (hours && minutes)
	cout << *hours << " " << *minutes << endl;*/
	return true;
}

void
setTimeSettings(int hours, int minutes, TIMES whatTime)
{
	times[whatTime].tm_hour = hours;
	times[whatTime].tm_min = minutes;
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
	int hours, minutes;
	for (size_t i = 0; i < TIMES::TIMES_SIZE; ++i)
	{
		do
		{
			cout << "When is your " << TIME_STRINGS[i * 2 + 1]
				<< " time in the 24-hour format HH:MM? ";
			cin >> input;
		} while (!isValidTime(input, &hours, &minutes));
		setTimeSettings(hours, minutes, static_cast<TIMES>(i));
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
		} while (!inputValid);
		setMaxEnergy(energy, static_cast<ENERGY_TYPE>(i));
	}

	cout << "First time setup complete." << endl;
}

int
main(int argc, char ** argv)
{
	int i;
	for (i = 0; i < argc; i++) {
		printf("Argument %i = %s\n", i, argv[i]);
	}

	firstTimeSetup();
	writeSettings();

	if (argc == 1)
	{
		//showPrompts();
	}
	else
	{

	}

	int x;
	cin >> x;

	return 0;
}