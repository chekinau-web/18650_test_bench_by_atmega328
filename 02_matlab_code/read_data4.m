clear; clc; close all;

filename = 'data.csv';
data = readmatrix(filename, 'NumHeaderLines', 1);
n = data(:,1);
current_raw = data(:,2);
voltage = data(:,3);

% Фильтрация
window_median = 41;
current_filtered = medfilt1(current_raw, window_median);

% Построение
figure('Position', [100 100 1200 600]);

subplot(2,1,1);
plot(n, current_raw, 'b.', 'MarkerSize', 3, 'DisplayName', 'Сырой ток');
hold on;
plot(n, current_filtered, 'r-', 'LineWidth', 1.5, 'DisplayName', 'Медиана 41');
grid on;
ylabel('Ток, А');
title('Фильтрация тока');
legend('Location', 'best');
xlim([n(1) n(end)]);

subplot(2,1,2);
plot(n, voltage, 'g-', 'LineWidth', 1.5);
grid on;
xlabel('Номер измерения (n), секунды');
ylabel('Напряжение, В');
title('Напряжение');
xlim([n(1) n(end)]);

% Статистика
fprintf('=== Статистика тока ===\n');
fprintf('Сырой: среднее = %.3f A, std = %.3f A\n', mean(current_raw), std(current_raw));
fprintf('Фильтрованный: среднее = %.3f A, std = %.3f A\n', mean(current_filtered), std(current_filtered));

exportgraphics(gcf, 'my_plot.png', 'Resolution', 300);