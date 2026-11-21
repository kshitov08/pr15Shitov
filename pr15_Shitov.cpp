// pr15_Shitov.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <fstream>
#include <Windows.h>
#include <ctime>
#include <string>
#include <climits>

using namespace std;

#define USE_MUTEX

const wchar_t* MUTEX_NAME = L"Local\\ShitovMutex";

struct BankAccount {
    int accountNumber;
    int balance;
};


void LogOperation(int pid, int initBalance, string opType, int amount, int finalBalance, string status) {
    ofstream log("operations.log", ios::app);
    if (log.is_open()) {
        log << pid << " | " << initBalance << " | " << opType << " | "
            << amount << " | " << finalBalance << " | " << status << "\n";
        log.close();
    }
}

void CheckAccount() {
    ifstream check("account.bin", ios::binary);
    if (!check.is_open()) {
        check.close();

        ofstream create("account.bin", ios::binary);
        BankAccount acc = { 10001, 10000 };
        create.write((char*)&acc, sizeof(BankAccount));
        create.close();
        cout << "Создан новый счет: 10001, Баланс: 10000\n";
    }
    else {
        check.close();
    }
}

//Чтение счета
BankAccount ReadAccount() {
    BankAccount acc;
    ifstream file("account.bin", ios::binary);
    file.read((char*)&acc, sizeof(BankAccount));
    file.close();
    return acc;
}

//Запись счета
void WriteAccount(BankAccount acc) {
    ofstream file("account.bin", ios::binary | ios::trunc);
    file.write((char*)&acc, sizeof(BankAccount));
    file.close();
}

//Выполнение операции
bool Operation(string opType, int amount) {
    int pid = GetCurrentProcessId();
    HANDLE hMutex = NULL;

#ifdef USE_MUTEX
    hMutex = CreateMutex(NULL, FALSE, MUTEX_NAME);
    if (hMutex == NULL) {
        cout << "Ошибка создания мьютекса!\n";
        return false;
    }

    DWORD waitResult = WaitForSingleObject(hMutex, 3000);

    if (waitResult == WAIT_ABANDONED) {
        cout << "Процесс аварийно завершился!\n";
        BankAccount checkAcc = ReadAccount();
        if (checkAcc.accountNumber != 10001 || checkAcc.balance < 0) {
            cout << "Данные счета повреждены! Восстановление...\n";
            checkAcc.accountNumber = 10001;
            checkAcc.balance = 10000;
            WriteAccount(checkAcc);
        }
        else {
            cout << "[ОК] Данные счета корректны.\n";
        }
    }
    else if (waitResult == WAIT_TIMEOUT) {
        cout << "Превышено время ожидания мьютекса\n";
        CloseHandle(hMutex);
        return false;
    }
#endif

    BankAccount acc = ReadAccount();
    int initBalance = acc.balance;
    string status = "OK";

#ifndef USE_MUTEX
    Sleep(200);
#endif

    if (opType == "Пополнение") {
        if (acc.balance > INT_MAX - amount) {
            status = "Переполнение!!!";
        }
        else {
            acc.balance += amount;
        }
    }
    else if (opType == "Снятие") {
        if (acc.balance < amount) {
            status = "Мало денег!!!";
        }
        else {
            acc.balance -= amount;
        }
    }

    if (status == "OK") {
        WriteAccount(acc);
    }
    int finalBalance = acc.balance;

    LogOperation(pid, initBalance, opType, amount, finalBalance, status);

#ifdef USE_MUTEX
    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
#endif



    cout << "PID: " << pid << " | " << opType << " " << amount
        << " руб. | Баланс: " << initBalance << " -> " << finalBalance
        << " | Статус: " << status << "\n";

    return (status == "OK");
}

int main() {
    setlocale(LC_ALL, "Russian");
    srand(GetTickCount());

    CheckAccount();

    int choice;
    do {
        cout << "\n===БАНКОВСКИЙ ТЕРМИНАЛ===\n";
        cout << "1. Просмотр баланса\n";
        cout << "2. Пополнить счет\n";
        cout << "3. Снять средства\n";
        cout << "4. Выполнить 20 случайных операций\n";
        cout << "0. Выход\n";
        cout << ">>> ";
        cin >> choice;

        if (choice == 1) {
            BankAccount acc = ReadAccount();
            cout << "Номер счета: " << acc.accountNumber << " | Баланс: " << acc.balance << " руб.\n";
        }
        else if (choice == 2) {
            int sum;
            cout << "Введите сумму для пополнения (>0): "; cin >> sum;
            if (sum > 0) Operation("Пополнение", sum);
            else cout << "Сумма должна быть положительной!\n";
        }
        else if (choice == 3) {
            int sum;
            cout << "Введите сумму для снятия (>0): "; cin >> sum;
            if (sum > 0) Operation("Снятие", sum);
            else cout << "Сумма должна быть положительной!\n";
        }
        else if (choice == 4) {
            cout << "\nЗапуск автоматических 20 операций...\n";
            for (int i = 0; i < 20; i++) {
                int amount = rand() % 901 + 100;
                int opType = rand() % 2;

                if (opType == 0) Operation("Пополнение", amount);
                else Operation("Снятие", amount);

                int delay = rand() % 401 + 100;
                Sleep(delay);
            }
            cout << "\n20 операций завершены.\n";
        }
    } while (choice != 0);

    return 0;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
