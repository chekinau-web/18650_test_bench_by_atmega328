% Очистка
clear; clc; close all;

% Загрузка данных
filename = 'data.csv';
data = readmatrix(filename, 'NumHeaderLines', 1);
n = data(:,1);
current_raw = data(:,2);
voltage = data(:,3);

% ========== ШАГ 1: УДАЛЕНИЕ ГРУБЫХ ВЫБРОСОВ ==========
mu = mean(current_raw);
sigma = std(current_raw);
outliers = abs(current_raw - mu) > 3*sigma;
current_clean = current_raw;
for i = find(outliers)'
    neighbors = current_raw(max(1,i-2):min(end,i+2));
    current_clean(i) = median(neighbors);
end

% ========== ШАГ 2: ФИЛЬТРАЦИЯ ==========
% Вариант А: Савицкий-Голей с помощью sgolayfilt (требует Signal Processing Toolbox)
window_sg = 11;   % длина окна (нечётная)
order_sg = 3;     % порядок полинома
if exist('sgolayfilt', 'file')
    current_filtered = sgolayfilt(current_clean, order_sg, window_sg);
else
    % Если нет Signal Processing Toolbox, используем медиану + movmean
    current_filtered = movmean(current_clean, 21);
end

% Вариант Б: Простое сильное скользящее среднее (окно 9)
% current_filtered = movmean(current_clean, 9);

% ========== ПОСТРОЕНИЕ ==========
%figure('Position', [100 100 1400 900]);
figure
subplot(3,1,1);
plot(n, current_raw, 'b.-', 'MarkerSize', 3);
grid on; title('Исходный ток'); ylabel('А');

subplot(3,1,2);
plot(n, current_filtered, 'r-', 'LineWidth', 2);
grid on; title('После фильтрации Савицкого-Голея (sgolayfilt)'); ylabel('А');

subplot(3,1,3);
plot(n, voltage, 'g-', 'LineWidth', 1.5);
grid on; title('Напряжение'); ylabel('В'); xlabel('n');

% Сравнение на одном графике
figure;
plot(n, current_raw, 'b.', 'MarkerSize', 3, 'DisplayName', 'Исходный');
hold on;
plot(n, current_filtered, 'r-', 'LineWidth', 2, 'DisplayName', 'Фильтрованный');
grid on; legend; title('Сравнение'); xlabel('n'); ylabel('Ток, А');