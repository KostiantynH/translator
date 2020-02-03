#include <fstream> //робота з файлами
#include <cmath>//математична
#include <Windows.h>//зміна кольру тексту в консолі
#include <clocale>//коректний вивід керилиці
#include <algorithm>//для функціїї replace
//#include <iomanip> // манипуляторы ввода/вывода
#include <string>
#include <iostream>
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);//отримання дескриптора пристрою стандартного виводу(необхідне для зміни кольору консолі)
std::string address = "D:\inProg.h87";//адреса вхыдного файлу
std::string input_txt;//вхідний тест програми на початковій мові
std::string Lexical_txt; //вхідний текст програми розбитий на лексеми
std::string Syntactic_txt;//містить текст програми що обробляється в синтаксичному аналізаторі
std::string Log_Syntactic; //проміжні результати згортки
std::string Asm_code;//містить згенерований код
std::string ID_table_name;//іменa ідентифікаторів
std::string ID_table_for_print;//таблиця ідентифікаторі разом із значенями (призначена виключно для друку в файл)
int ID_table[255][3];//перший елемент - ім'я в числовому еквіваленті, другий - значеня, третій - показує чи змінна ініціалізована
bool correct_line[9999] = { 0 };// містить інформацію про пидаленя симіолв (необхідно для коректного виводу рядка помилки)
int check_token(std::string token, int state_automat, int last_state_automat, int symbol, int position, int automat_number)//перевіряє символ на відповідність лексемі(це автомат-функція)
{
	if (last_state_automat < 0)//автомат розпізнавання ключових слів
	{
		if (input_txt[symbol] == token[symbol - position]){ state_automat = -1; }
		else { state_automat = 0; }
		if (symbol - position == (token.length() - 1) && state_automat == -1){ state_automat = automat_number; }
	}
	return state_automat;
}
void Error_handler(int code, int line)//(видаленя проміжних файлів)
{
	for (int i = 0; i < line; i++, line = line + correct_line[i]){}//обраховує номер рядка у вхідному коді
	system("cls");//очищення консолі
	std::cout << "line: " << line << std::endl;
	if (code < 100)std::cout << "Error: ";
	if (code == 100){ std::cout << "Lexical error." << std::endl; }
	if (code == 200){ std::cout << "Syntax error." << std::endl; }
	if (code == 1){ std::cout << "unidentified symbol." << std::endl; }
	if (code == 2){ std::cout << "incorrect conclusion comments." << std::endl; }
	if (code == 3){ std::cout << "constant value too big." << std::endl; }
	if (code == 4){ std::cout << "incorrect conclusion constant." << std::endl; }
	if (code == 5){ std::cout << "incorrect beginning announcement constant." << std::endl; }
	if (code == 10){ std::cout << "repeated announcement identifier." << std::endl; }
	if (code == 11){ std::cout << "undeclared identifier." << std::endl; }
	if (code == 12){ std::cout << "No label end of the program." << std::endl; }
	if (code == 13){ std::cout << "incorrect use operator input|output." << std::endl; }
	if (code == 14){ std::cout << "incorrect using comparison operator." << std::endl; }
	if (code == 15){ std::cout << "unexpected error." << std::endl; }
	if (code == 16){ std::cout << "program can contain only one label beginning." << std::endl; }
	if (code == 17){ std::cout << "program can contain only one data block." << std::endl; }
	if (code == 18){ std::cout << "program can contain only one label end." << std::endl; }
	if (code == 19){ std::cout << "incorrect beginning comments." << std::endl; }
	if (code == 20){ std::cout << "incorrect announcement the loop." << std::endl; }
	system("pause");
	exit(1);
}
void Saving(std::string Name, std::string txt)//записує данні в файл за вказаним ім'ям за адресою файлу вхідного тексту програми
{
	std::string correct_address;
	correct_address.append(address);
	for (; correct_address.back() != '.';)
	{
		correct_address.pop_back();
	}
	correct_address.append(Name);
	std::ofstream output_file(correct_address); // створюємо об'єкт класу ofstream для запису до фалй
	output_file << txt;//запис данних до файду
	output_file.close(); // закриття файлу
}
void Read(std::string address)//зчитує вмітиме файлу в стрінг та знаходть його овжину
{
	std::ifstream input_file_txt(address); // створюємо об'єкт класу ifstream для зчитування даних із файлу input.txt
	if (input_file_txt.is_open()==0){std::cout << "\n\nError: cannot open file\n\n";system("pause");exit(1);}//перевірка чи файл відкрився
	getline(input_file_txt, input_txt, '\0'); //читання вмісту файлу
	input_file_txt.close(); // закриття файлу
}
void Delete_coments()
{
	for (int z = 0, line = 1, check = 0, position = 0, length = 0; z < input_txt.length() && input_txt.length() != 0;)
	{
		if (input_txt[z] != '%' && check == 1) { Error_handler(19, line); }
		if (input_txt[z] != '%' && check == 3) { Error_handler(2, line); }

		if (input_txt[z] == '%' && check == 3){ check = 4; }
		if (input_txt[z] == '%' && check == 2){ check = 3; }
		if (input_txt[z] == '%' && check == 1){ check = 2; position = z - 1; length = 4; }
		if (input_txt[z] == '%' && check == 0){ check = 1; }

		if (input_txt[z] == '\n'){ line++; }
		if (input_txt[z] == '\n' && check == 2){ correct_line[line] = 1; }

		if (check != 4){ z++; }
		if (check == 4)
		{

			input_txt.erase(position, length);
			z = z - (length - 1);
			check = 0;
			position = 0;
			length = 0;
		}

		if (check == 2 && input_txt[z] != '%'){ length++; }
	}
}
void Lexical_analysis()
{
	int length_input_txt;//довжина вхідної послідовності символів(коду)
	int line = 1; //рядок вхідного коду що аналізується
	int current_symbol = 0;//позиція символа що аналізується
	int current_position = 0;//позиція початку аналізу
	int const avtomat = 32;//кількість автоматів в лексичному аналізаторі
	int state_automat[avtomat];//соточні стани автомата
	int last_state_automat[avtomat];//попередні стани автомата
	std::string const_buff;//накопичує константу при лексичному аналізі
	std::string identification_buff;//накопичує ідентифікатор при лексичному аналізі
	int last_automat_sum = 1;//необхідно для першого входження в цикл
	length_input_txt = input_txt.length();
	for (; current_position < length_input_txt;)//йде від початку до кінця коду
	{
		int automat_sum = 1;//необхідно для першого входження в цикл
		for (int ll = 0; ll < avtomat; ll++)//як заповнити масив по іншому?????
		{
			state_automat[ll] = { -2 };
			last_state_automat[ll] = { -2 };
		}

		for (current_symbol = current_position; automat_sum != 0 && current_symbol <= length_input_txt; current_symbol++)//цикл йде від позиції початку лексеми доки не зламаються всі автомати
		{
			for (int ii = 0; ii < avtomat; ii++){ last_state_automat[ii] = state_automat[ii]; state_automat[ii] = 0; }//збереженя попередніх станів автоматів та зануленя поточних станів автоматів
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			///////////////////////////////////////////////////////автомати//////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			if (input_txt[current_symbol] == ' ' || input_txt[current_symbol] == '\t')//запобігає повторенню пробілів та табуляцій
			{
				for (bool n = 1; n == 1 && current_symbol < length_input_txt;)
				{
					if (input_txt[current_symbol + 1] == ' ' || input_txt[current_symbol + 1] == '\t'){ current_symbol++; }
					else{ n = 0; }
				}
			}

			for (; input_txt[current_symbol] == '\n' && input_txt[current_symbol + 1] == '\n'; current_symbol++)//запобігає повтореням символу переходу на нову стрічку
			{
				int line2 = line; for (int i = 0; i < line2; i++, line2 = line2 + correct_line[i]){}correct_line[line2] = 1;
			}//фіксує видаленя переходу на нову стрічку

			if (last_state_automat[29] == -2)
			{
				for (; input_txt[current_symbol] == '0' && input_txt[current_symbol + 1] == '0'; current_symbol++){}//запобігає повтореням нулів
			}


			if (last_state_automat[29] != 0 && 1 < (input_txt.length() - current_symbol))//автомат констант
			{
				if (input_txt[current_symbol] >= '0' && input_txt[current_symbol] <= '9')
				{
					state_automat[29] = 29;
					const_buff.push_back(input_txt[current_symbol]);
				}
				else
				{
					state_automat[29] = 0;
				}
				if (last_state_automat[29] == -2 && state_automat[29] == 29)//перевіряє чи правилно почато оголошеня константи
				{
					bool j = 0;
					if (Lexical_txt.back() == '['){ Lexical_txt.push_back('+'); }
					if (Lexical_txt[(Lexical_txt.length()) - 1] == '+'&&Lexical_txt[(Lexical_txt.length()) - 2] == '['){ j = 1; }
					if (Lexical_txt[(Lexical_txt.length()) - 1] == '-'&&Lexical_txt[(Lexical_txt.length()) - 2] == '['){ j = 1; }
					if (j == 1){}
					else{ Error_handler(5, line); }
				}

				if (last_state_automat[29] == 29 && state_automat[29] == 0)//перевіряє корекність завершеня константи
				{
					int r = 0;
					for (int h = current_symbol; r == 0 && h < input_txt.length(); h++)
					{
						if (input_txt[h] != ' '){ r = -1; }
						if (input_txt[h] == ']'){ r = 1; }
					}
					if (r < 0){ state_automat[29] = 0; const_buff.clear(); Error_handler(4, line); }
				}
				if (input_txt[current_symbol + 1] == ']' || input_txt[current_symbol + 1] == ' ')
				{
					if (state_automat[29] != 0)
					{
						state_automat[29] = 29;
						unsigned int sum = 0;
						for (int f = 0; f < const_buff.length(); f++)//первірки чи константа не перевищує 4 байти
						{
							sum = sum + ((int)(const_buff[f] - 48)) * (pow(10, (const_buff.length() - f - 1)));
						}
						if (sum > 2147483647){ Error_handler(3, line); }
						std::string buff;//проміжний буфер для нормалізації форми константи
						for (int f = const_buff.length(); f < 10; f++){ buff.push_back('0'); }//цикл нормалізації форми константи
						buff = buff + const_buff;
						const_buff = buff;
					}
				}



			}

			if (last_state_automat[30] < 0)// автомат ідентифікаторів
			{
				identification_buff.push_back(input_txt[current_symbol]);

				if (last_state_automat[30] == -2)
				{
					if (input_txt[current_symbol] >= 'A' && input_txt[current_symbol] <= 'Z'){ state_automat[30] = -1; }
					else{ state_automat[30] = 0; identification_buff.clear(); }
				}
				if (last_state_automat[30] == -1)
				{
					if (input_txt[current_symbol] >= 'a' && input_txt[current_symbol] <= 'z'){ state_automat[30] = -1; }
					else{ state_automat[30] = 0; identification_buff.clear(); }
				}
				if (identification_buff.length() == 7){ state_automat[30] = 30; }

				if (identification_buff.length() < 7)
				{
					if (input_txt[current_symbol + 1] >= 'a' && input_txt[current_symbol + 1] <= 'z'){ state_automat[30] = -1; }
					else{ state_automat[30] = 0; identification_buff.clear(); }
				}
			}

			state_automat[0] = check_token("MainProgram", state_automat[0], last_state_automat[0], current_symbol, current_position, 1);
			state_automat[1] = check_token("StartData", state_automat[1], last_state_automat[1], current_symbol, current_position, 2);
			state_automat[2] = check_token("Int32_t", state_automat[2], last_state_automat[2], current_symbol, current_position, 3);
			state_automat[3] = check_token("Output", state_automat[3], last_state_automat[3], current_symbol, current_position, 4);
			state_automat[4] = check_token("Input", state_automat[4], last_state_automat[4], current_symbol, current_position, 5);
			state_automat[5] = check_token("For", state_automat[5], last_state_automat[5], current_symbol, current_position, 6);
			state_automat[6] = check_token("And", state_automat[6], last_state_automat[6], current_symbol, current_position, 7);
			state_automat[7] = check_token("Mul", state_automat[7], last_state_automat[7], current_symbol, current_position, 8);
			state_automat[8] = check_token("Div", state_automat[8], last_state_automat[8], current_symbol, current_position, 9);
			state_automat[9] = check_token("Mod", state_automat[9], last_state_automat[9], current_symbol, current_position, 10);
			state_automat[10] = check_token("Or", state_automat[10], last_state_automat[10], current_symbol, current_position, 11);
			state_automat[11] = check_token("End", state_automat[11], last_state_automat[11], current_symbol, current_position, 12);
			state_automat[12] = check_token("::", state_automat[12], last_state_automat[12], current_symbol, current_position, 13);
			state_automat[13] = check_token("!", state_automat[13], last_state_automat[13], current_symbol, current_position, 14);
			state_automat[14] = check_token("+", state_automat[14], last_state_automat[14], current_symbol, current_position, 15);
			state_automat[15] = check_token("-", state_automat[15], last_state_automat[15], current_symbol, current_position, 16);
			state_automat[16] = check_token(";", state_automat[16], last_state_automat[16], current_symbol, current_position, 17);
			state_automat[17] = check_token("=", state_automat[17], last_state_automat[17], current_symbol, current_position, 18);
			state_automat[18] = check_token("<=", state_automat[18], last_state_automat[18], current_symbol, current_position, 19);
			state_automat[19] = check_token(">=", state_automat[19], last_state_automat[19], current_symbol, current_position, 20);
			state_automat[20] = check_token("{", state_automat[20], last_state_automat[20], current_symbol, current_position, 21);
			state_automat[21] = check_token("}", state_automat[21], last_state_automat[21], current_symbol, current_position, 22);
			state_automat[22] = check_token("(", state_automat[22], last_state_automat[22], current_symbol, current_position, 23);
			state_automat[23] = check_token(")", state_automat[23], last_state_automat[23], current_symbol, current_position, 24);
			state_automat[24] = check_token("[", state_automat[24], last_state_automat[24], current_symbol, current_position, 25);
			state_automat[25] = check_token("]", state_automat[25], last_state_automat[25], current_symbol, current_position, 26);
			state_automat[26] = check_token("\n", state_automat[26], last_state_automat[26], current_symbol, current_position, 27);
			state_automat[27] = check_token(" ", state_automat[27], last_state_automat[27], current_symbol, current_position, 28);
			state_automat[28] = check_token("\t", state_automat[28], last_state_automat[28], current_symbol, current_position, 28);
			state_automat[31] = check_token("<>", state_automat[31], last_state_automat[31], current_symbol, current_position, 31);

			automat_sum = 0;
			for (int i = 0; i < avtomat; i++){ automat_sum = automat_sum + state_automat[i]; }//перевірки чи зламались всі автомати(шляхом сумування всіх станів з метою подальшого аналізу суми)
			if (automat_sum == 0){ current_symbol--; }
			if (last_automat_sum <= 0 && automat_sum == 0){ Error_handler(1, line); }
			last_automat_sum = automat_sum;

			//for (int i = 0; i < avtomat; i++){printf("%2d", state_automat[i]);}printf("%4d", automat_sum);printf("%6d", current_symbol);std::cout << "    " << input_txt[current_symbol] << std::endl;//DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_
			//system("pause");//DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//запис проміжної форми у простих лексемах//////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		automat_sum = 0;
		for (int i = 0; i < avtomat; i++){ automat_sum = automat_sum + last_state_automat[i]; }//визначаємо який автомат завершив роботу пере поломкою
		//std::cout <<  std::endl;//DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_
		if (automat_sum == 1){ Lexical_txt.push_back('M'); }//MainProgram
		if (automat_sum == 2){ Lexical_txt.push_back('S'); }//StartData
		if (automat_sum == 3){ Lexical_txt.push_back('T'); }//Int32_t
		if (automat_sum == 4){ Lexical_txt.push_back('O'); }//Output
		if (automat_sum == 5){ Lexical_txt.push_back('I'); }//Input
		if (automat_sum == 6){ Lexical_txt.push_back('F'); }//For
		if (automat_sum == 7){ Lexical_txt.push_back('&'); }//And
		if (automat_sum == 8){ Lexical_txt.push_back('*'); } // Mul
		if (automat_sum == 9){ Lexical_txt.push_back('/'); }//Div
		if (automat_sum == 10){ Lexical_txt.push_back('#'); }//Mod
		if (automat_sum == 11){ Lexical_txt.push_back('|'); }//Or
		if (automat_sum == 12){ Lexical_txt.push_back('E'); }//End
		if (automat_sum == 13){ Lexical_txt.push_back(':'); }//::
		if (automat_sum == 14) { Lexical_txt.push_back('!'); }//
		if (automat_sum == 15) { Lexical_txt.push_back('+'); }//
		if (automat_sum == 16) { Lexical_txt.push_back('-'); }//
		if (automat_sum == 17) { Lexical_txt.push_back(';'); }//
		if (automat_sum == 18) { Lexical_txt.push_back('='); }//
		if (automat_sum == 19) { Lexical_txt.push_back('<'); }//
		if (automat_sum == 20) { Lexical_txt.push_back('>'); }//
		if (automat_sum == 21) { Lexical_txt.push_back('{'); }//
		if (automat_sum == 22) { Lexical_txt.push_back('}'); }//
		if (automat_sum == 23) { Lexical_txt.push_back('('); }//
		if (automat_sum == 24) { Lexical_txt.push_back(')'); }//
		if (automat_sum == 25) { Lexical_txt.push_back('['); }//
		if (automat_sum == 26) { Lexical_txt.push_back(']'); }//
		if (automat_sum == 27) { Lexical_txt.push_back('\n'); line++; }//// символ пеерходу на нову стрічку
		if (automat_sum == 28) { /*Lexical_txt.push_back(' '); */ }//// символ "Spase" або "Tab"
		if (automat_sum == 29) { Lexical_txt.append(const_buff); }//виконує запис констант
		if (automat_sum == 30) { Lexical_txt.push_back('~'); Lexical_txt.append(identification_buff); Lexical_txt.push_back('~'); }//виконує запис ідентифікаторів(та заключає їх керуючий символ "~")
		if (automat_sum == 31) { Lexical_txt.push_back('N'); }//<> (операція "не дорівнює")
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		identification_buff.clear();//очистка буферу ідентифікаторів аби не накопичувати непотрібні чссла
		const_buff.clear();//очистка буферу констано аби не накопичувати непотрібні чссла
		current_position = (current_symbol);//зміна позиції символа з якого почнеться аанліз наступної лексеми
	}

	for (int b = 1; b < Lexical_txt.length(); b++)//доставляє пропущені символи ";" (якщо рядок закінчується не ними)
	{
		if (Lexical_txt[b] == '\n')
		{
			switch (Lexical_txt[b - 1])
			{
			case'E':{ break; }//не ставить ";" в кінці стрічки 
			case'M':{ break; }//не ставить ";" в кінці стрічки 
			case'S':{ break; }//не ставить ";" в кінці стрічки 
			case';':{ break; }//не ставить ";" в кінці стрічки 
			case'{':{ break; }//не ставить ";" в кінці стрічки 
			case'}':{ break; }//не ставить ";" в кінці стрічки 
			case'(':{ break; }//не ставить ";" в кінці стрічки 
			case')':{ break; }//не ставить ";" в кінці стрічки 
			default:{ Lexical_txt.insert(b, ";"); break; }//ставить ";" в кінці стрічки 
			}
		}
	}
	for (int b = 1, f = 1; b < Lexical_txt.length(); b++)//видаляє подвоєння символу ";;"
	{
		if (Lexical_txt[b - 1] == '('){ f = 0; }
		if (Lexical_txt[b - 1] == ')'){ f = 1; }
		if (f == 1)
		{
			if (Lexical_txt[b - 1] == ';' && Lexical_txt[b] == ';'){ Lexical_txt.erase(b, 1); b--; }
		}
	}
	Syntactic_txt = Lexical_txt;//запис для подальшого аналізу
}
void Create_ID_table()
{
	int current_symbol = Lexical_txt.length();//позиція символа що аналізується
	int current_line = 1; //рядок вхідного коду що аналізується
	std::string name_buff;//буфер імен
	std::string check_buff;//буфер перевірки імен
	int kk = 0;//дозвіл запису в таблицю(перевіряє чи ми знаходимось в блоці оголошеня зміних)
	for (; current_symbol > 0; current_symbol--)
	{
		if (Lexical_txt[current_symbol] == '\n'){ current_line++; }//?????????????????????????????????????????????????????????????????????
		if (Lexical_txt[current_symbol - 1] != '~'&&Lexical_txt[current_symbol] == 'S'){ kk = -1; }
		if (Lexical_txt[current_symbol - 1] != '~'&&Lexical_txt[current_symbol] == 'E'){ kk = 1; }
		if (kk == 1 && current_symbol > 7)
		{
			bool k = 0;
			if (Lexical_txt[current_symbol] == '~' && Lexical_txt[current_symbol - 8] == '~'){ k = 1; }//розпізнає початок ідентифікатора за ключовим символом
			if (Lexical_txt[current_symbol - 7] < 'A' || Lexical_txt[current_symbol - 7] > 'Z'){ k = 0; }//перевыряє коректність запису ідентифікатора
			if (Lexical_txt[current_symbol - 6] < 'a' || Lexical_txt[current_symbol - 6] > 'z'){ k = 0; }//перевыряє коректність запису ідентифікатора
			if (Lexical_txt[current_symbol - 5] < 'a' || Lexical_txt[current_symbol - 5] > 'z'){ k = 0; }//перевыряє коректність запису ідентифікатора
			if (Lexical_txt[current_symbol - 4] < 'a' || Lexical_txt[current_symbol - 4] > 'z'){ k = 0; }//перевыряє коректність запису ідентифікатора
			if (Lexical_txt[current_symbol - 3] < 'a' || Lexical_txt[current_symbol - 3] > 'z'){ k = 0; }//перевыряє коректність запису ідентифікатора
			if (Lexical_txt[current_symbol - 2] < 'a' || Lexical_txt[current_symbol - 2] > 'z'){ k = 0; }//перевыряє коректність запису ідентифікатора
			if (Lexical_txt[current_symbol - 1] < 'a' || Lexical_txt[current_symbol - 1] > 'z'){ k = 0; }//перевыряє коректність запису ідентифікатора
			for (int s = 7; k == 1 && s > 0; s--){ name_buff.push_back(Lexical_txt[current_symbol - s]); }//записує знайдений ідентифікатор

			if (ID_table_name.length() != 0 && k == 1)
			{
				for (int f = 0; (f + 7) <= ID_table_name.length(); f = f + 7)//перевіряє чи є вже такий ідентифікаторв в таблиці імен
				{
					check_buff.push_back(ID_table_name[f]);
					check_buff.push_back(ID_table_name[f + 1]);
					check_buff.push_back(ID_table_name[f + 2]);
					check_buff.push_back(ID_table_name[f + 3]);
					check_buff.push_back(ID_table_name[f + 4]);
					check_buff.push_back(ID_table_name[f + 5]);
					check_buff.push_back(ID_table_name[f + 6]);
					if (check_buff == name_buff){ Error_handler(10, current_line); }//?????????????????????????????????????????????????????????????????????некоректно виводить адресу помилки
					check_buff.clear();
				}
			}
			if (k == 1)
			{
				ID_table_name.append(name_buff);
				int summ = 0;
				k = 0;
				if (Lexical_txt[current_symbol + 1] == ':' && Lexical_txt[current_symbol + 2] == '[')
				{
					if (Lexical_txt[current_symbol + 3] == '+' || Lexical_txt[current_symbol + 3] == '-'){ k = 1; }
				}
				ID_table_for_print.append(name_buff);//ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_
				ID_table_for_print.push_back('\t');//ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_
				ID_table_for_print.append(" <>  ");//ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_
				if (k == 1)
				{
					ID_table_for_print.push_back(Lexical_txt[current_symbol + 3]);//ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_
					for (int g = 4; g < 14; g++)
					{
						ID_table_for_print.push_back((Lexical_txt[current_symbol + g]));//ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_
						summ = summ + ((Lexical_txt[current_symbol + g]) - 48)* (pow(10, (13 - g)));
					}
					if ((Lexical_txt[current_symbol + 3]) == '-'){ summ = summ *(-1); }
					ID_table[(ID_table[0][0]) + 1][1] = summ;
					ID_table[(ID_table[0][0]) + 1][2] = 1;
					ID_table[0][0]++;
				}
				else
				{
					ID_table_for_print.append("uninitialized");//ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_
					ID_table[(ID_table[0][0]) + 1][2] = 0;
					ID_table[0][0]++;
				}
				ID_table_for_print.push_back('\n');//ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_ForPrint_

			}
			name_buff.clear();
		}
		if (kk == -1 && current_symbol > 7)
		{
			bool k = 0;
			if (Lexical_txt[current_symbol] == '~' && Lexical_txt[current_symbol - 8] == '~'){ k = 1; }//розпізнає початок ідентифікатора за ключовим символом
			if (Lexical_txt[current_symbol - 7] < 'A' || Lexical_txt[current_symbol - 7] > 'Z'){ k = 0; }//перевыряє коректність запису ідентифікатора
			if (Lexical_txt[current_symbol - 6] < 'a' || Lexical_txt[current_symbol - 6] > 'z'){ k = 0; }//перевыряє коректність запису ідентифікатора
			if (Lexical_txt[current_symbol - 5] < 'a' || Lexical_txt[current_symbol - 5] > 'z'){ k = 0; }//перевыряє коректність запису ідентифікатора
			if (Lexical_txt[current_symbol - 4] < 'a' || Lexical_txt[current_symbol - 4] > 'z'){ k = 0; }//перевыряє коректність запису ідентифікатора
			if (Lexical_txt[current_symbol - 3] < 'a' || Lexical_txt[current_symbol - 3] > 'z'){ k = 0; }//перевыряє коректність запису ідентифікатора
			if (Lexical_txt[current_symbol - 2] < 'a' || Lexical_txt[current_symbol - 2] > 'z'){ k = 0; }//перевыряє коректність запису ідентифікатора
			if (Lexical_txt[current_symbol - 1] < 'a' || Lexical_txt[current_symbol - 1] > 'z'){ k = 0; }//перевыряє коректність запису ідентифікатора
			for (int s = 7; k == 1 && s > 0; s--){ name_buff.push_back(Lexical_txt[current_symbol - s]); }//записує знайдений ідентифікатор

			for (int f = 0; (f + 7) <= ID_table_name.length() && k == 1; f = f + 7)//перевіряє чи є вже такий ідентифікаторв в таблиці імен
			{
				check_buff.push_back(ID_table_name[f]);
				check_buff.push_back(ID_table_name[f + 1]);
				check_buff.push_back(ID_table_name[f + 2]);
				check_buff.push_back(ID_table_name[f + 3]);
				check_buff.push_back(ID_table_name[f + 4]);
				check_buff.push_back(ID_table_name[f + 5]);
				check_buff.push_back(ID_table_name[f + 6]);
				if (check_buff == name_buff){ k = 0; }
				//std::cout << check_buff << "\t" << name_buff << "\t"<<k << std::endl;//DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_
				check_buff.clear();
			}
			if (k == 1){ Error_handler(11, current_line); }//?????????????????????????????????????????????????????????????????????некоректно виводить адресу помилки
			name_buff.clear();
		}
	}
}
void Convolution(std::string rule, int symbol_ASCII)
{
	std::string symbol;
	symbol.push_back((char)(symbol_ASCII));
	int h = rule.length() - 1;
	for (; Syntactic_txt.find(rule) > 0 && Syntactic_txt.find(rule) < Syntactic_txt.length() - h;)
	{
		Syntactic_txt.insert(Syntactic_txt.find(rule), symbol);
		Syntactic_txt.erase(Syntactic_txt.find(rule), rule.length());
	}

}
void Convolution8(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int symbol_ASCII)
{
	std::string symbol;
	symbol.push_back((char)(symbol_ASCII));
	std::string rule;
	if (a1 >= 0){ rule.push_back((char)(a1)); }
	if (a2 >= 0){ rule.push_back((char)(a2)); }
	if (a3 >= 0){ rule.push_back((char)(a3)); }
	if (a4 >= 0){ rule.push_back((char)(a4)); }
	if (a5 >= 0){ rule.push_back((char)(a5)); }
	if (a6 >= 0){ rule.push_back((char)(a6)); }
	if (a7 >= 0){ rule.push_back((char)(a7)); }
	if (a8 >= 0){ rule.push_back((char)(a8)); }
	int h = rule.length() - 1;
	for (; Syntactic_txt.find(rule) > 0 && Syntactic_txt.find(rule) < Syntactic_txt.length() - h;)
	{
		Syntactic_txt.insert(Syntactic_txt.find(rule), symbol);
		Syntactic_txt.erase(Syntactic_txt.find(rule), rule.length());
	}

}
void Syntactic_analysis()
{
	int line = 1; //рядок вхідного коду що аналізується
	int current_position = 0;//позиція початку аналізу
	int current_rule = 0;//вказує яке правило застосовується на данному проході по тексті
	for (; current_rule < 12; current_rule++)
	{
		if (current_rule == 1)//згортає мультиплікативні операції операції у "NUL"<0>
		{
			Convolution("*", 0);
			Convolution("/", 0);
			Convolution("#", 0);
		}
		if (current_rule == 1)//згортає логічні операції операції у "SOH"<1>
		{
			Convolution("|", 1);
			//Convolution("!", 1);
			Convolution8(-1, -1, -1, -1, -1, -1, 33, 64, 12);//"!@"
			Convolution8(-1, -1, -1, -1, -1, -1, 33, 63, 12);//"!?"
			Convolution("&", 1);
		}
		if (current_rule == 1)//згортає адетивні операції операції у "STX"<2>
		{
			Convolution("+", 2);
			Convolution("-", 2);
		}
		if (current_rule == 1)//згортає операції порівняння у "ETX"<3>
		{
			Convolution("<", 3);
			Convolution(">", 3);
			Convolution("N", 3);
			Convolution("=", 3);
		}
		if (current_rule == 1)//згортає оператори вводу\виводу у "EOT"<4>
		{
			Convolution("O?", 4);
			Convolution("I?", 4);
		}

		if (current_rule == 3)//згортає вираз порівняння "ENQ"<5>
		{
			Convolution8(-1, -1, -1, -1, -1, 64, 3, 64, 5);
			Convolution8(-1, -1, -1, -1, -1, 63, 3, 63, 5);
			Convolution8(-1, -1, -1, -1, -1, 63, 3, 64, 5);
			Convolution8(-1, -1, -1, -1, -1, 64, 3, 63, 5);
		}
		if (current_rule == 4)//згортає блок змінних "ASK"<6>
		{
			Convolution("T?:@", 6);//T ? : @;
			Convolution("T?", 6);//T ? ;
		}
		if (current_rule == 5)//згортає оператор присвоювання у проміжну форму "^"<94> (щоб змінні яким не присвоюється значеня можна було згортати як даданки)
		{
			Convolution("?:", 94);
		}

		if (current_rule == 6)//згортає доданок у "BS"<8>
		{
			Convolution8(-1, -1, -1, -1, -1, 64, 0, 64, 8);
			Convolution8(-1, -1, -1, -1, -1, 63, 0, 63, 8);
			Convolution8(-1, -1, -1, -1, -1, 63, 0, 64, 8);
			Convolution8(-1, -1, -1, -1, -1, 64, 0, 63, 8);
			Convolution8(-1, -1, -1, -1, -1, 64, 2, 64, 8);
			Convolution8(-1, -1, -1, -1, -1, 63, 2, 63, 8);
			Convolution8(-1, -1, -1, -1, -1, 63, 2, 64, 8);
			Convolution8(-1, -1, -1, -1, -1, 64, 2, 63, 8);
			Convolution("?", 8);
		}
		if (current_rule == 7)//згортає вираз у "FF"<12>
		{

			Convolution8(-1, -1, -1, -1, -1, 8, 0, 8, 12);
			Convolution8(-1, -1, -1, -1, -1, 64, 1, 64, 12);
			Convolution8(-1, -1, -1, -1, -1, 63, 1, 63, 12);
			Convolution8(-1, -1, -1, -1, -1, 63, 1, 64, 12);
			Convolution8(-1, -1, -1, -1, -1, 64, 1, 63, 12);
			Convolution8(-1, -1, -1, -1, -1, 8, 2, 8, 12);
			replace(Syntactic_txt.begin(), Syntactic_txt.end(), (char)(8), (char)(12));
			Convolution8(-1, -1, -1, -1, -1, 40, 12, 41, 12);//(вираз)>>вирваз
			Convolution8(-1, -1, -1, -1, -1, 12, 0, 12, 12);
			Convolution8(-1, -1, -1, -1, -1, 12, 1, 12, 12);
			Convolution8(-1, -1, -1, -1, -1, 12, 2, 12, 12);
		}
		if (current_rule == 8)//згортає оператор присвоювання у "DLE"<16>
		{
			Convolution8(94, 12, -1, -1, -1, -1, -1, -1, 16);
			Convolution("^@", 16);
		}
		if (current_rule == 9)//згортає оператор циклу "DC1"<17> (але не згортає тіло циклу)
		{

			Convolution8(70, 40, -1, 59, -1, 59, -1, 41, 17);// F(;;)
			Convolution8(70, 40, -1, 59, -1, 59, 16, 41, 17);// F(;;DLE)
			Convolution8(70, 40, -1, 59, -1, 59, 12, 41, 17);// F(;;FF)
			Convolution8(70, 40, -1, 59, 5, 59, -1, 41, 17);// F(;ENQ;)
			Convolution8(70, 40, -1, 59, 5, 59, 16, 41, 17);// F(;ENQ;DLE)
			Convolution8(70, 40, -1, 59, 5, 59, 12, 41, 17);// F(;ENQ;FF)
			Convolution8(70, 40, -1, 59, 16, 59, -1, 41, 17);// F(;DLE;)
			Convolution8(70, 40, -1, 59, 16, 59, 16, 41, 17);// F(;DLE;DLE)
			Convolution8(70, 40, -1, 59, 16, 59, 12, 41, 17);// F(;DLE;FF)
			Convolution8(70, 40, -1, 59, 12, 59, -1, 41, 17);// F(;FF;)
			Convolution8(70, 40, -1, 59, 12, 59, 16, 41, 17);// F(;FF;DLE)
			Convolution8(70, 40, -1, 59, 12, 59, 12, 41, 17);// F(;FF;FF)	
			Convolution8(70, 40, 16, 59, -1, 59, -1, 41, 17);// F(DLE;;)
			Convolution8(70, 40, 16, 59, -1, 59, 16, 41, 17);// F(DLE;;DLE)
			Convolution8(70, 40, 16, 59, -1, 59, 12, 41, 17);// F(DLE;;FF)
			Convolution8(70, 40, 16, 59, 5, 59, -1, 41, 17);// F(DLE;ENQ;)
			Convolution8(70, 40, 16, 59, 5, 59, 16, 41, 17);// F(DLE;ENQ;DLE)
			Convolution8(70, 40, 16, 59, 5, 59, 12, 41, 17);// F(DLE;ENQ;FF)
			Convolution8(70, 40, 16, 59, 16, 59, -1, 41, 17);// F(DLE;DLE;)
			Convolution8(70, 40, 16, 59, 16, 59, 16, 41, 17);// F(DLE;DLE;DLE)
			Convolution8(70, 40, 16, 59, 16, 59, 12, 41, 17);// F(DLE;DLE;FF)
			Convolution8(70, 40, 16, 59, 12, 59, -1, 41, 17);// F(DLE;FF;)
			Convolution8(70, 40, 16, 59, 12, 59, 16, 41, 17);// F(DLE;FF;DLE)
			Convolution8(70, 40, 16, 59, 12, 59, 12, 41, 17);// F(DLE;FF;FF)
		}

		if (current_rule == 10)//видаляє оператори (якщо вони у блоках в яких мають бути)
		{
			//видаленя не несе функціональної небезпеки так як блоки можуть бути порожніми
			for (int s = Syntactic_txt.find("M"); s < Syntactic_txt.find("S"); s++)//блок операторів
			{
				if (Syntactic_txt[s] == (char)(16)){ Syntactic_txt.erase(s, 1); }//оператор присвоєння
				if (Syntactic_txt[s] == (char)(4)){ Syntactic_txt.erase(s, 1); }//оператор вводу\виводу

			}
			for (int s = Syntactic_txt.find("S"); s < Syntactic_txt.find("E"); s++)
			{
				if (Syntactic_txt[s] == (char)(6)){ Syntactic_txt.erase(s, 1); }//блок данних
			}

			for (int i = 0; i + 2 < Syntactic_txt.length(); i++)//перетворює  DC1{  в  DC1"\n"{     (це необхідно для наступної згортки)
			{
				if (Syntactic_txt[i] == ((char)(17)) && Syntactic_txt[i + 2] == '{')
				{
					Syntactic_txt.erase(i + 1, 1);
					Syntactic_txt.insert(i + 2, "\n");
					i = 0;
				}
			}
			Convolution8(17, 123, -1, -1, -1, -1, -1, -1, 18);// DC1{  згоратє в DC2

			for (int a, k = 0, i = 0, l = 1; i < Syntactic_txt.length(); i++)//завершує видаленя операторів а саме видаляє дужки
			{
				if (Syntactic_txt[i] == '\n'){ l++; }
				if (Syntactic_txt[i] == ((char)(18))){ a = i; k = 1; }
				if (Syntactic_txt[i] == '}' && k == 1)
				{
					Syntactic_txt.erase(i, 1);
					Syntactic_txt.erase(a, 1);
					k = 0;
					i = 0;
				}
			}
		}



		if (current_rule == 11)//згортає програму
		{
			line = 1;
			for (int state = 0, i = 0; i < Syntactic_txt.length(); i++)
			{

				switch (Syntactic_txt[i])
				{
				case'\n':{ line++;  break; }
				case'M':{if (state == 0){ state = 1; } else{ Error_handler(16, line); }  break; }
				case'S':{if (state == 1){ state = 2; } else{ Error_handler(17, line); }  break; }
				case'E':{if (state == 2){ state = 3; } else{ Error_handler(18, line); }  break; }
				case';':{ break; }
				case ((char)(18)) : {Error_handler(20, line); break; }

				default:{ Error_handler(200, line); break; }
				}

			}
		}

		for (current_position = 0; current_position < Syntactic_txt.length(); current_position++)//йде від початку до кінця коду
		{

			if (current_rule == 0 && (current_position + 6) < Syntactic_txt.length())//згортає змінні у  символ "?" <63>
			{
				bool k = 0;
				if (Syntactic_txt[current_position] == '~' && Syntactic_txt[current_position + 8] == '~'){ k = 1; }//розпізнає початок ідентифікатора за ключовим символом
				if (Syntactic_txt[current_position + 1] < 'A' || Syntactic_txt[current_position + 1] > 'Z'){ k = 0; }//перевыряє коректність запису ідентифікатора
				if (Syntactic_txt[current_position + 2] < 'a' || Syntactic_txt[current_position + 2] > 'z'){ k = 0; }//перевыряє коректність запису ідентифікатора
				if (Syntactic_txt[current_position + 3] < 'a' || Syntactic_txt[current_position + 3] > 'z'){ k = 0; }//перевыряє коректність запису ідентифікатора
				if (Syntactic_txt[current_position + 4] < 'a' || Syntactic_txt[current_position + 4] > 'z'){ k = 0; }//перевыряє коректність запису ідентифікатора
				if (Syntactic_txt[current_position + 5] < 'a' || Syntactic_txt[current_position + 5] > 'z'){ k = 0; }//перевыряє коректність запису ідентифікатора
				if (Syntactic_txt[current_position + 6] < 'a' || Syntactic_txt[current_position + 6] > 'z'){ k = 0; }//перевыряє коректність запису ідентифікатора
				if (Syntactic_txt[current_position + 7] < 'a' || Syntactic_txt[current_position + 7] > 'z'){ k = 0; }//перевыряє коректність запису ідентифікатора
				if (k == 1){ Syntactic_txt.erase(current_position, 9); Syntactic_txt.insert(current_position, "?"); }
			}


			if (current_rule == 0 && (current_position + 10) < Syntactic_txt.length())//згортає константи у  символ "@" <64>
			{

				bool k = 0;
				if (Syntactic_txt[current_position] == '[' && Syntactic_txt[current_position + 12] == ']'){ k = 1; }//розпізнає початок ідентифікатора за ключовим символом
				for (int o = 2; o < 12; o++)
				{
					if (Syntactic_txt[current_position + o] < '0' || Syntactic_txt[current_position + o] > '9'){ k = 0; }
				}
				if (k == 1){ Syntactic_txt.erase(current_position, 13); Syntactic_txt.insert(current_position, "@"); }
			}

		}
		//////////////////////////////////запис до файлу///////////////////////////////////////
		Log_Syntactic.push_back('\n');
		Log_Syntactic.append("---------------------------------------");
		if (current_rule < 10){ Log_Syntactic.push_back(((char)(current_rule)) + 48); }
		else{ Log_Syntactic.push_back('1'); Log_Syntactic.push_back(((char)(current_rule - 9)) + 48); }
		if (current_rule < 10){ Log_Syntactic.append("---------------------------------------"); }
		else{ Log_Syntactic.append("--------------------------------------"); }
		Log_Syntactic.push_back('\n');
		Log_Syntactic.append(Syntactic_txt);
		///////////////////////////////////////////////////////////////////////////////////////
	}
}
std::string ShuntingYard_algorithm(std::string in/*вхідний вираз*/)
{
	std::string out;//результат перетвореня
	std::string steck;//стек операцій
	int Priority = -1;//приоритет

	for (int i = 0; i < in.length(); i++, Priority = -1)
	{
		if (in[i] == '~'){ for (int j = 0; j < 9; j++){ out.push_back(in[j + i]); }  i = i + 9; }//variabl
		if (in[i] == '['){ for (int j = 0; j < 13; j++){ out.push_back(in[j + i]); }  i = i + 13; break; } //const
		switch (in[i])
		{
		case'+':{ Priority = 3; break; }//адетивна
		case'-':{ Priority = 3; break; }//адетивна
		case'*':{ Priority = 4; break; }//мультиплікативна
		case'/':{ Priority = 4; break; }//мультиплікативна
		case'#':{ Priority = 4; break; }//мультиплікативна
		case'&':{ Priority = 4; break; }//логічна
		case'|':{ Priority = 4; break; }//логічна
		case'!':{ Priority = 4; break; }//логічна
		case'=':{ Priority = 2; break; }//порівняння
		case'N':{ Priority = 2; break; }//порівняння
		case'<':{ Priority = 2; break; }//порівняння
		case'>':{ Priority = 2; break; }//порівняння
		case'(':{ Priority = 0; break; }
		case')':{ Priority = 1; break; }
		default:{ break; }
		}

		if (Priority >= 0 && steck.length() > 0)
		{
			for (int Pri, d = steck.length() - 1; d >= 0; d--)
			{
				switch (steck[d])
				{
				case'+':{ Pri = 2; break; }//адетивна
				case'-':{ Pri = 2; break; }//адетивна
				case'*':{ Pri = 3; break; }//мультиплікативна
				case'/':{ Pri = 3; break; }//мультиплікативна
				case'#':{ Pri = 3; break; }//мультиплікативна
				case'&':{ Pri = 3; break; }//логічна
				case'|':{ Pri = 3; break; }//логічна
				case'!':{ Pri = 3; break; }//логічна
				case'=':{ Pri = 4; break; }//порівняння
				case'N':{ Pri = 4; break; }//порівняння
				case'<':{ Pri = 4; break; }//порівняння
				case'>':{ Pri = 4; break; }//порівняння
				case'(':{ Pri = 0; break; }
				case')':{ Pri = 1; break; }
				default:{ break; }
				}
				if (Priority < Pri)
				{
					out.push_back(steck[d]);
					steck.erase(d, 1);
				}
			}
			steck.push_back(in[i]);
		}

		if (Priority >= 0 && steck.length() == 0)
		{
			steck.push_back(in[i]);
		}

	}
	for (int o = steck.length() - 1; o >= 0; o--){ out.push_back(steck[o]); }
	for (int t = 0; t < out.length();){ if (out[t] == '(' || out[t] == ')'){ out.erase(t, 1); } else{ t++; } }//видаляє дужки
	return out;
}
std::string Code_generation(std::string boof)//генерує асемблерний код виразу що в нього передається
{
	std::string consumer;//містить ім'я змінної якій необхідно присвоїти значеня виразу
	if (boof.length()>8)
	{
		for (int i = 1; i < 8; i++){ consumer.push_back(boof[i]); }
		boof.erase(0, 10);
	}
	std::string expression = ShuntingYard_algorithm(boof);//мітить вираз інфіксної нотації (ЗПЗ)

	std::string code;
	for (int h = 0; h < expression.length(); h++)
	{
		switch (expression[h])
		{
		case'~':{code.append("fild ");  for (int j = 1; j < 8; j++){ code.push_back(expression[j + h]); }; code.append("\n"); h = h + 8; break; }//variabl
		case'[':{code.append("mov boofc, ");  for (int j = 1; j < 12; j++){ code.push_back(expression[j + h]); }; code.append("\nfild boofc\n"); h = h + 12; break; } //const
		case'+':{code.append("fadd\n"); break; }//адетивна
		case'-':{code.append("fsub\n"); break; }//адетивна
		case'*':{code.append("fmul\n"); break; }//мультиплікативна
		case'/':{code.append("fdiv\n"); break; }//мультиплікативна
		case'#':{code.append("fistp\tboofb\nfistp\tboofa\nxor\tedx, edx\nmov\teax, boofa\nmov\tecx, boofb\ndiv\tecx\nmov\tboofc, edx\nfild\tboofc\n"); break; }//мультиплікативна	
		case'&':{code.append("fistp\tboofb\nfistp\tboofa\nxor\tedx, edx\nmov\teax, boofa\nmov\tecx, boofb\nand\teax, ecx\nmov\tboofc, eax\nfild\tboofc\n");  break; }//логічна
		case'|':{code.append("fistp\tboofb\nfistp\tboofa\nxor\tedx, edx\nmov\teax, boofa\nmov\tecx, boofb\nor\teax, ecx\nmov\tboofc, eax\nfild\tboofc\n"); break; }//логічна
		case'!':{code.append("fchs\n"); break; }//логічна 
			//case'=':{ break; }//порівняння
			//case'N':{ break; }//порівняння
			//case'<':{ break; }//порівняння
			//case'>':{ break; }//порівняння
		default:{ break; }
		}

	}
	code.append("fistp ");
	code.append(consumer);
	code.append("\n");
	return code;
}
void Code_generator()
{
	std::string end_of_the_for;//містить закінчення циклу "for"
	int position = 0;//позиція початку аналізу

	std::string Worck_txt = Lexical_txt;
	for (int p = 0, k = 0; p + 1 < Worck_txt.length();)//видаляє секцію оголошення даних (так як вона заповнюється окремо з таблиці ідентифікаторів)
	{
		if (Worck_txt[p] == 'S' && Worck_txt[p + 1] == ';') { k = 1; p = p + 3; }
		if (Worck_txt[p] == 'S' && Worck_txt[p + 1] == '\n'){ k = 1; p = p + 2; }
		if (Worck_txt[p] == 'E'){ k = 0; }
		if (k == 1){ Worck_txt.erase(p, 1); }
		else{ p++; }
	}

	std::string head = ".586\n"
		".model flat, STDCALL, c\n"
		"include \\masm32\\include\\masm32rt.inc; необхідна для функцій 'crt_scanf' і 'printf'\n";

	std::string data = ".data\n"
		"   scnId\tdb\t'%d', 0; формат вводу\n"
		"   boofa\tdd\t0;\n"
		"   boofb\tdd\t0;\n"
		"   boofc\tdd\t0;\n";

	for (int f = 0; (f + 7) <= ID_table_name.length(); f = f + 7)//записує оголошені змінні до .data сигменту
	{
		data.append("   ");
		data.push_back(ID_table_name[f]);
		data.push_back(ID_table_name[f + 1]);
		data.push_back(ID_table_name[f + 2]);
		data.push_back(ID_table_name[f + 3]);
		data.push_back(ID_table_name[f + 4]);
		data.push_back(ID_table_name[f + 5]);
		data.push_back(ID_table_name[f + 6]);
		data.append("\tdd\t");
		data.append(std::to_string(ID_table[f / 7 + 1][1]));
		data.append(";\n");
	}

	std::string code = ".code\n"
		"start:\n";
	std::string end = "invoke crt__getch\n"
		"exit\n"
		"end start\n";

	for (; position < Worck_txt.length(); position++)//проходить по всьому тексту
	{
		if (position + 8 < Worck_txt.length())//генерує код оператора вводу
		{
			if (Worck_txt[position] == 'I' && Worck_txt[position + 1] == '~')
			{
				code.append("printf(");
				code.push_back(((char)(34)));//ставить подвійні лапки лапки
				code.append("Enter ");
				for (int p = 2; p < 9; p++){ code.push_back(Worck_txt[position + p]); }
				code.append(": ");
				code.push_back(((char)(34)));//ставить подвійні лапки лапки
				code.append(")\nlea eax,");
				for (int p = 2; p < 9; p++){ code.push_back(Worck_txt[position + p]); }
				code.append("\ninvoke crt_scanf, offset scnId, eax; \n");
				position = position + 10;
			}

		}

		if (position + 8 < Worck_txt.length())//генерує код оператора виводу
		{
			if (Worck_txt[position] == 'O' && Worck_txt[position + 1] == '~')
			{
				code.append("printf(");
				code.push_back(((char)(34)));//ставить подвійні лапки лапки
				for (int p = 2; p < 9; p++){ code.push_back(Worck_txt[position + p]); }
				code.append(" is: %d");
				code.push_back(((char)(92)));//ставить "\"
				code.push_back('n');
				code.push_back(((char)(34)));//ставить подвійні лапки лапки
				code.append(", ");
				for (int p = 2; p < 9; p++){ code.push_back(Worck_txt[position + p]); }
				code.append(")\n");
				position = position + 10;
			}
		}

		if (position < Worck_txt.length())//генерує код код циклу"for"
		{
			if (Worck_txt[position] == 'F' && Worck_txt[position + 1] == '(')
			{
				std::string op1;
				std::string op2;
				std::string op3;
				position = position + 2;

				for (int g = 0; position < Worck_txt.length() && Worck_txt[position] != ')'; position++)
				{

					switch (Worck_txt[position])
					{
					case';':{g++; break; }
					default:
					{
							   if (Worck_txt[position] == ';'){ g++; position++; }
							   if (g == 0){ op1.push_back(Worck_txt[position]); }
							   if (g == 1)
							   {
								   op2.push_back(Worck_txt[position]);
								   if (Worck_txt[position] == 'N'){ op2.pop_back(); op2.append("!="); }
								   if (Worck_txt[position] == '='){ op2.append("="); }
							   }
							   if (g == 2){ op3.push_back(Worck_txt[position]); }
							   break;
					}
					}

				}
				replace(op2.begin(), op2.end(), '~', ' ');
				replace(op2.begin(), op2.end(), '[', ' ');
				replace(op2.begin(), op2.end(), ']', ' ');

				if (op1.length() > 8){ code.append(Code_generation(op1)); }
				code.append(".WHILE ");
				code.append(op2);
				code.append("\n");
				end_of_the_for.append(op3);
				end_of_the_for.append("|");
			}
		}

		if (Worck_txt[position] == '}')//зробити так щоб ерок уиклу обчислювався перед його закінчкня
		{
			std::string new_back;
			std::string g;
			end_of_the_for.pop_back();
			for (; end_of_the_for.length() > 0 && end_of_the_for.back() != '|';)
			{
				g = end_of_the_for.back();
				new_back.insert(0, g);
				end_of_the_for.pop_back();
			}
			code.append(Code_generation(new_back));
			code.append(".ENDW\n"); 
		}

		if (position + 9 < Worck_txt.length())//генерує код присвоєння змінній виразу
		{
			if (Worck_txt[position] == '~' && Worck_txt[position + 9] == ':')
			{
				std::string boof;
				for (; Worck_txt[position] != ';' && Worck_txt[position] != '\n'; position++)//зчитує вираз
				{
					boof.push_back(Worck_txt[position]);
				}
				code.append(Code_generation(boof));
			}
		}

	}
	Asm_code.append(head);
	Asm_code.append(data);
	Asm_code.append(code);
	Asm_code.append(end);
}

int main()
{
	//setlocale(LC_CTYPE, "ukr");//зі стрінго чомсь не канає :(
	std::cout << "Please specify the location of the file. \t Example: D: / inProg.h87\nAddress: ";
	//std::cin >> address;
	Read(address);
	Delete_coments();
	Lexical_analysis();
	Saving("Lexical", Lexical_txt);//збереженя результатів лексичного аналізу у файл
	std::cout << "\n\n\n\nLexical analysis\t\tcomplete\n";
	Create_ID_table();
	Saving("ID_table", ID_table_for_print);//збереженя результатів стоврення таблиці ідентифікаторів у файл
	Syntactic_analysis();
	Saving("Log", Log_Syntactic);//збереженя результатів синтаксичного аналізу у файл
	std::cout << "Syntactic analysis\t\tcomplete\n";
	Code_generator();
	Saving("asm", Asm_code);
	std::cout << "Assembler code generation\tcomplete\n\n";
	//std::cout<< std::endl << "------------------------------------input_txt----------------------------------" << std::endl;//DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_
	//std::cout << input_txt << std::endl;//DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_
	//std::cout<< std::endl << "-----------------------------------Lexical_txt---------------------------------" << std::endl;//DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_
	//std::cout << Lexical_txt << std::endl;//DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_
	//std::cout << "------------------------------------Asm_code-----------------------------------" << std::endl;//DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_
	//std::cout<< std::endl << Asm_code<< std::endl;//DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_DEBUG_
	system("pause");
	return 0;
}