#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex mutex;                 // Мьютекс для синхронизации потоков
std::condition_variable condition; // Условная переменная для сигнализации о событии

int eventFlag = 0; // Флаг события

// Поток-поставщик
void producer() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Задержка в одну секунду

        {
            std::unique_lock<std::mutex> lock(mutex); // Захватываем мьютекс

            eventFlag = 1; // Устанавливаем флаг события
            std::cout << "Поставщик: отправлено событие" << std::endl;

            condition.notify_one(); // Сообщаем потребителю о событии
        } // Автоматически освобождаем мьютекс при выходе из блока
    }
}

// Поток-потребитель
void consumer() {
    while (true) {
        std::unique_lock<std::mutex> lock(mutex); // Захватываем мьютекс

        while (eventFlag == 0) {
            condition.wait(lock); // Ожидаем события и освобождаем мьютекс
        }

        eventFlag = 0; // Сбрасываем флаг события
        std::cout << "Потребитель: получено событие" << std::endl;
    }
}

int main() {
    std::thread producer_thread(producer);
    std::thread consumer_thread(consumer);

    producer_thread.join(); // Ждем завершения потока-поставщика
    consumer_thread.join(); // Ждем завершения потока-потребителя

    return 0;
}
