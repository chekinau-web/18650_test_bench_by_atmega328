% Очистка рабочего пространства
clear; clc; close all;

% Данные (скопируйте из вашего сообщения в файл, например 'data.csv')
% Предполагается, что файл data.csv находится в текущей папке.
% Формат: n,Current_A,Voltage_V,ADC_current,ADC_voltage
filename = 'data.csv';

% Импорт данных
data = readmatrix(filename, 'NumHeaderLines', 1); % пропускаем заголовок
n = data(:,1);          % номер измерения (время)
current = data(:,2);    % ток, А
voltage = data(:,3);    % напряжение, В

% Построение графиков
figure('Name', 'Анализ тока и напряжения', 'NumberTitle', 'off');

% Подграфик тока
subplot(2,1,1);
plot(n, current, 'b-', 'LineWidth', 1.5);
grid on;
xlabel('Номер измерения (n)');
ylabel('Ток, А');
title('Изменение тока во времени');
xlim([n(1) n(end)]);

% Подграфик напряжения
subplot(2,1,2);
plot(n, voltage, 'r-', 'LineWidth', 1.5);
grid on;
xlabel('Номер измерения (n)');
ylabel('Напряжение, В');
title('Изменение напряжения во времени');
xlim([n(1) n(end)]);

% % Дополнительно: совместный график с двумя осями Y (опционально)
% figure('Name', 'Ток и напряжение на одном графике', 'NumberTitle', 'off');
% yyaxis left
% plot(n, current, 'b-', 'LineWidth', 1.5);
% ylabel('Ток, А');
% yyaxis right
% plot(n, voltage, 'r-', 'LineWidth', 1.5);
% ylabel('Напряжение, В');
% grid on;
% xlabel('Номер измерения (n)');
% title('Сравнение тока и напряжения');
% legend('Ток', 'Напряжение', 'Location', 'best');

