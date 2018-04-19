#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

using namespace std;

struct Data {
	const size_t LIMIT;
	double *values;
	double *macds;
	double *signals;

	double wallet;
	double stocks;
	double lastValue;

	Data(size_t limit) : LIMIT(limit) {
		values = new double[limit];
		macds = new double[limit];
		signals = new double[limit];

		wallet = 1000;
		stocks = 0;
	}

	~Data() {
		delete[] values;
		delete[] macds;
		delete[] signals;
	}
};

bool is_Strong(const Data& data, size_t i) {
	return true;
	//return fabs(data.macds[i]) >= 0.05;
}

bool should_Buy(const Data& data, size_t i) {
	return (i >= 1 && data.macds[i - 1] < data.signals[i - 1] && data.macds[i] > data.signals[i] && is_Strong(data, i));
}

bool should_Sell(const Data& data, size_t i) {
	return (i >= 1 && data.macds[i - 1] > data.signals[i - 1] && data.macds[i] < data.signals[i] && is_Strong(data, i));
}

double calc_EMA(const double* arr, size_t index_start, size_t N) {

	// Przekroczenie zakresu
	if (index_start < N) {
		N = index_start;
	}

	// 2 najprostszy przypadki (lim)
	if (N <= 1) {
		return arr[index_start - N]; // N=0 -> p0, N=1 -> p1
	}	

	const double alpha = 1.0 - (2.0 / (N - 1));
	double up = arr[index_start]; // p0
	double down = 1;

	for (size_t counter = 1, index = (index_start-1); counter <= N; counter++, index--) {
		const double helper = pow(alpha, counter);
		up += (arr[index] * helper);
		down += helper;
	}

	return up / down;
}

int main(int argc, char* argv[]) {

	srand(time(NULL));

	// Odzyt argumentów i otwarcie pliku

	if (argc != 2 && argc != 4 && argc != 6) {
		cout << "MACD <input> [-out <output>] [-limit <limit>]" << endl;
		return 1;
	}

	// Parametry domyœlne
	string input_file_path = argv[1];
	string output_file_path = "./result.txt";
	size_t limit = 1000;

	for (int i = 2; i < (argc-1); i+=2) {
		string arg = argv[i];
		if (arg == "-out") {
			output_file_path = argv[i+1];
		}
		else if (arg == "-limit") {
			limit = atoi(argv[i+1]);
		}
	}

	if (limit == 0) {
		cout << "Nieprawidlowy limit !" << endl;
		return 1;
	}

	ifstream input_file(input_file_path);
	if (!input_file) {
		cout << "Problem z otwarciem plikiu " << input_file_path << endl;
		return 1;
	}

	// Odczyt z pliku
	Data data(limit);
	string buffor;
	for (size_t i = 0; i < data.LIMIT; i++) {
		if (!getline(input_file, buffor)) {
			input_file.close();
			cout << "Zbyt malo danych (" << i << "/" << data.LIMIT << ")" << endl;
			return 1;
		}

		try {
			data.values[i] = stod(buffor);
		}
		catch (...) {
			input_file.close();
			cout << "Dane w lini " << (i + 1) << " nie jest liczba !" << endl;
			return 1;
		}
	}
	input_file.close();

	// Pierwsze obliczenia i zapis do pliku

	ofstream output_file(output_file_path);
	if (!output_file) {
		cout << "Problem z plikiem wynikowym !" << endl;
		return 1;
	}

	double R = 0.11;
	
	for (size_t i = 0; i < data.LIMIT; i++) {
		data.macds[i] = calc_EMA(data.values, i, 12) - calc_EMA(data.values, i, 26);
		data.signals[i] = calc_EMA(data.macds, i, 9);

		if (data.wallet > 0 && (should_Buy(data, i) || data.values[i]/data.lastValue <= 0.90)) { 
			// Kupno zgodnie z MACD lub gdy zysk 10%
			data.stocks = data.wallet / data.values[i];
			data.wallet = 0;
			data.lastValue = data.values[i];
		}
		else if (data.stocks > 0 && should_Sell(data, i) && (data.values[i]/data.lastValue >= 0.96 || data.values[i] / data.lastValue <= 0.85) ) {
			// Sprzeda¿ zgodnei z MACD pod warunkiem maksymalnie 4% starty lub straty powy¿ej 15% (wyjœcie ewakuacyjne)
			data.wallet = data.stocks * data.values[i];
			data.stocks = 0;
			data.lastValue = data.values[i];
		}

		output_file << fixed << data.values[i] << '\t' << data.macds[i] << '\t' << data.signals[i] << '\t' << endl;
	}
	output_file.close();

	if (data.stocks > 0) {
		data.wallet = data.stocks * data.values[data.LIMIT - 1];
		data.stocks = 0;
	}

	double procent = 100 * ((data.wallet / 1000.0) - 1);
	cout << "Konto: " << data.wallet << " (" << procent << "%)" << endl;
	cout << "Poprawnie odczytano, przetworzono i zapisano " << data.LIMIT << " danych" << endl;

	return 0;
}