% Очистка рабочего пространства
clear; clc; close all;

% ========== ЗАГРУЗКА ДАННЫХ ==========
filename = 'data.csv';
data = readmatrix(filename, 'NumHeaderLines', 1); % пропускаем заголовок
n = data(:,1);          % номер измерения (время)
current_raw = data(:,2);    % ток, А (сырой)
voltage = data(:,3);    % напряжение, В

% ========== ФИЛЬТРАЦИЯ ТОКА ==========
% Параметры фильтров
window_median = 31;      % окно медианного фильтра (нечётное)
window_average = 7;     % окно скользящего среднего

% 1. Медианный фильтр (удаляет импульсные выбросы)
current_median = medfilt1(current_raw, window_median);

% 2. Скользящее среднее (сглаживание)
current_smooth = movmean(current_median, window_average);

% 3. (Опционально) Низкочастотный фильтр Баттерворта
fs = 1;                 % частота дискретизации (1 Гц, т.к. измерения раз в секунду)
fc = 0.05;               % частота среза (Гц) – подберите под вашу задачу
[b, a] = butter(4, fc/(fs/2), 'low');
current_filtered = filtfilt(b, a, current_median); % нулевая фаза

% Выберите один из вариантов для дальнейшего построения:
%current_clean = current_smooth;   % используем скользящее среднее после медианы
%current_clean = current_filtered; % или фильтр Баттерворта
current_clean = current_median;
% ========== ПОСТРОЕНИЕ ГРАФИКОВ ==========
%figure('Name', 'Анализ тока и напряжения', 'NumberTitle', 'off', 'Position', [100 100 1200 800]);
figure
% График 1: Сырой ток
subplot(2,1,1);
plot(n, current_raw, 'b.-', 'LineWidth', 1, 'MarkerSize', 4);
grid on;
ylabel('Ток, А (сырой)');
title('Исходные данные');
xlim([n(1) n(end)]);

% График 2: Отфильтрованный ток (после медианы + сглаживание)
%subplot(3,1,2);
hold on
plot(n, current_clean, 'r-', 'LineWidth', 1.5);
grid on;
ylabel('Ток, А (фильтрованный)');
title('После медианного фильтра и скользящего среднего');
xlim([n(1) n(end)]);

% График 3: Напряжение
subplot(2,1,2);
plot(n, voltage, 'g-', 'LineWidth', 1.5);
grid on;
xlabel('Номер измерения (n)');
ylabel('Напряжение, В');
title('Напряжение');
xlim([n(1) n(end)]);

% Дополнительно: сравнение сырого и фильтрованного тока на одном графике
figure('Name', 'Сравнение фильтрации', 'NumberTitle', 'off');
plot(n, current_raw, 'b.', 'MarkerSize', 3, 'DisplayName', 'Сырой ток');
hold on;
plot(n, current_clean, 'r-', 'LineWidth', 1.5, 'DisplayName', 'Фильтрованный ток');
grid on;
xlabel('Номер измерения (n)');
ylabel('Ток, А');
legend('Location', 'best');
title('Эффект фильтрации (медиана + moving average)');
hold off;

% ========== СТАТИСТИКА ДО И ПОСЛЕ ==========
fprintf('=== Статистика тока ===\n');
fprintf('Сырой: среднее = %.3f A, std = %.3f A\n', mean(current_raw), std(current_raw));
fprintf('Фильтрованный: среднее = %.3f A, std = %.3f A\n', mean(current_clean), std(current_clean));