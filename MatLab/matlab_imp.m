% === Load STL model ===
model = stlread('colleague_model.stl');
tri = triangulation(model.ConnectivityList, model.Points);
normals = faceNormal(tri);
center = mean(model.Points, 1);  % Center of the model

% === Setup Figure ===
figure;
h = patch('Faces', model.ConnectivityList, ...
          'Vertices', model.Points, ...
          'FaceColor', 'flat', ...
          'EdgeColor', 'none');
axis equal;
xlabel('X'); ylabel('Y'); zlabel('Z');
title('Sun Movement in 12 Steps (Half-Day Cycle)');

% Custom colormap: dark to bright yellow
yellowMap = [linspace(0.3, 1, 256)', linspace(0.3, 1, 256)', zeros(256,1)];
colormap(yellowMap);

lighting gouraud;
camlight('headlight');
colorbar;
rotate3d on;

% === View Settings ===
sunRadius = 200;  % Distance of sun
view(3);
axis([-100 400 -100 400 -50 300]);  % Expanded axis to see sun

% === Sun Movement Settings ===
nSteps = 12;
pauseTime = 0.2;
angles = linspace(0, pi, nSteps);  % From East to West

% === Points to Track ===
pts = [149 138 10; 185 176 10; 159 264 10];
labels = {'Point 1', 'Point 2', 'Point 3'};
colors = [1 0 0; 0 1 0; 0 0 1];  % Red, Green, Blue
intensity_at_pts = zeros(nSteps, 3);

% === Plot Points on Model ===
hold on;
for i = 1:3
    plot3(pts(i,1), pts(i,2), pts(i,3), 'o', ...
        'MarkerSize', 8, 'MarkerFaceColor', colors(i,:), ...
        'MarkerEdgeColor', 'k');
end
hold off;

% === First Pass: Calculate Lighting and Store Intensity ===
for i = 1:nSteps
    theta = angles(i);

    sunX = sunRadius * cos(theta);
    sunY = 0;
    sunZ = sunRadius * sin(theta);
    sunPos = center + [sunX, sunY, sunZ];
    sunDir = sunPos - center;
    sunDir = sunDir / norm(sunDir);

    % Compute lighting
    intensity = dot(normals, repmat(sunDir, size(normals,1), 1), 2);
    intensity = max(intensity, 0);
    set(h, 'FaceVertexCData', intensity);

    % Track intensity at each point
    for j = 1:3
        dists = vecnorm(model.Points - pts(j,:), 2, 2);
        [~, idx] = min(dists);
        faceIDs = find(any(model.ConnectivityList == idx, 2));
        avgNormal = mean(normals(faceIDs,:), 1);
        avgNormal = avgNormal / norm(avgNormal);
        dotVal = max(dot(avgNormal, sunDir), 0);
        intensity_at_pts(i, j) = dotVal;
    end

    % Plot Sun
    hold on;
    sun = plot3(sunPos(1), sunPos(2), sunPos(3), 'yo', ...
        'MarkerSize', 10, 'MarkerFaceColor', 'y');
    hold off;

    caxis([0 1]);
    drawnow;
    pause(pauseTime);
    if isvalid(sun), delete(sun); end
end

% === Plot Graphs ===
timeLabels = ["06:00 AM", "07:00 AM", "08:00 AM", "09:00 AM", ...
              "10:00 AM", "11:00 AM", "12:00 PM", "01:00 PM", ...
              "02:00 PM", "03:00 PM", "04:00 PM", "05:00 PM"];

t = linspace(0, 1, 100);  % Simulated time axis
brightYellow = [1, 1, 0];
darkYellow   = [0.3, 0.3, 0];

for j = 1:3
    figure;
    hold on;
    for i = 1:nSteps
        Lnode = intensity_at_pts(i, j);
        Ltarget = -(Lnode)^2 + 1;
        y = Ltarget * ones(size(t));
        lineColor = darkYellow + (brightYellow - darkYellow) * Lnode;

        plot(t, y, 'Color', lineColor, 'LineWidth', 2, ...
             'DisplayName', timeLabels(i));
    end
    title(['L_{target}(t) for ' labels{j} ' [' num2str(pts(j,1)) ', ' num2str(pts(j,2)) ', ' num2str(pts(j,3)) ']']);
    xlabel('Simulated Time');
    ylabel('L_{target} = -L_{node}^2 + 1');
    legend('Location', 'eastoutside');
    grid on;
    axis([0 1 0 1.1]);
    box on;
end

% === Infinite Loop to Keep Sun Rotating ===
while true
    for i = 1:nSteps
        theta = angles(i);

        sunX = sunRadius * cos(theta);
        sunY = 0;
        sunZ = sunRadius * sin(theta);
        sunPos = center + [sunX, sunY, sunZ];
        sunDir = sunPos - center;
        sunDir = sunDir / norm(sunDir);

        % Lighting
        intensity = dot(normals, repmat(sunDir, size(normals,1), 1), 2);
        intensity = max(intensity, 0);
        set(h, 'FaceVertexCData', intensity);

        % Plot sun
        hold on;
        sun = plot3(sunPos(1), sunPos(2), sunPos(3), 'yo', ...
            'MarkerSize', 10, 'MarkerFaceColor', 'y');
        hold off;

        caxis([0 1]);
        drawnow;
        pause(0.2);
        if isvalid(sun), delete(sun); end
    end
end
