fname = "../build/orocos/rtReport.dat"; 
fid = fopen(fname); 
raw = fread(fid,inf); 
str = char(raw'); 
fclose(fid); 
val = jsondecode(str);

amountOfData = size(val.root);
amountOfData = amountOfData(1);

updateContainer_CS1 = [];
updateContainer_S = [];
updateContainer_R = [];
updateContainer_CS2 = [];
updateContainer_SSS = [];

read_new_R = [];
read_old_R = [];
write_SSS = [];
write_S = [];
write_CS1 = [];
write_CS2 = [];

hatch_SSS_X = [];
hatch_SSS_Y = [];

hatch_S_X = [];
hatch_S_Y = [];

hatch_R_X = [];
hatch_R_Y = [];

annotations = {};

for index = 1:amountOfData
    item = val.root(index);
    if strcmp(item.call_name, 'updateHook()') == 1
        if strcmp(item.container_name, 'CS1') == 1
             %disp(item)
            tmp=[double(item.call_time), 0; double(item.call_time), 1; double(item.call_duration), 1; double(item.call_duration), 0];
            updateContainer_CS1 = cat(1, updateContainer_CS1, tmp);
        elseif strcmp(item.container_name, 'S') == 1
            tmp=[double(item.call_time), 0; double(item.call_time), 0.8; double(item.call_duration), 0.8; double(item.call_duration), 0];
            hatch_S_X = cat(1, hatch_S_X, [double(item.call_time), double(item.call_time), double(item.call_duration), double(item.call_duration)]);
            hatch_S_Y = cat(1, hatch_S_Y, [0,0.8,0.8,0]);
            updateContainer_S = cat(1, updateContainer_S, tmp);
        elseif strcmp(item.container_name, 'R') == 1
            tmp=[double(item.call_time), 0; double(item.call_time), 0.8; double(item.call_duration), 0.8; double(item.call_duration), 0];
            hatch_R_X = cat(1, hatch_R_X, [double(item.call_time), double(item.call_time), double(item.call_duration), double(item.call_duration)]);
            hatch_R_Y = cat(1, hatch_R_Y, [0,0.8,0.8,0]);
            updateContainer_R = cat(1, updateContainer_R, tmp);
       elseif strcmp(item.container_name, 'CS2') == 1
            tmp=[double(item.call_time), 0; double(item.call_time), 1; double(item.call_duration), 1; double(item.call_duration), 0];
            updateContainer_CS2 = cat(1, updateContainer_CS2, tmp);
        elseif strcmp(item.container_name, 'SSS') == 1
            tmp=[double(item.call_time), 0; double(item.call_time), 0.8; double(item.call_duration), 0.8; double(item.call_duration), 0];
            hatch_SSS_X = cat(1, hatch_SSS_X, [double(item.call_time), double(item.call_time), double(item.call_duration), double(item.call_duration)]);
            hatch_SSS_Y = cat(1, hatch_SSS_Y, [0,0.8,0.8,0]);
            updateContainer_SSS = cat(1, updateContainer_SSS, tmp);
        end
    elseif strcmp(item.call_type, 'CALL_PORT_READ_NEWDATA') == 1
        if strcmp(item.container_name, 'R') == 1
            tmp=[double(item.call_time), 0.8];
            annotations = cat(1, annotations, {tmp(1), tmp(2), strcat(item.container_name,'.',item.call_name,' NewData')});
            read_new_R = cat(1, read_new_R, tmp);
        end
    elseif strcmp(item.call_type, 'CALL_PORT_READ_OLDDATA') == 1
        if strcmp(item.container_name, 'R') == 1
            tmp=[double(item.call_time), 0.5];
            annotations = cat(1, annotations, {tmp(1), tmp(2), strcat(item.container_name,'.',item.call_name,' OldData')});
            read_old_R = cat(1, read_old_R, tmp);
        end
    elseif strcmp(item.call_type, 'CALL_PORT_WRITE') == 1
        if strcmp(item.container_name, 'SSS') == 1
            tmp=[double(item.call_time), 0.2];
            annotations = cat(1, annotations, {tmp(1), tmp(2), strcat(item.container_name,'.',item.call_name,' Write')});
            write_SSS = cat(1, write_SSS, tmp);
        elseif strcmp(item.container_name, 'S') == 1
            tmp=[double(item.call_time), 0.2];
            annotations = cat(1, annotations, {tmp(1), tmp(2), strcat(item.container_name,'.',item.call_name,' Write')});
            write_S = cat(1, write_S, tmp);
        elseif strcmp(item.container_name, 'CS1') == 1
            tmp=[double(item.call_time), 0.2];
            annotations = cat(1, annotations, {tmp(1), tmp(2), strcat(item.container_name,'.',item.call_name,' Write')});
            write_CS1 = cat(1, write_CS1, tmp);
        elseif strcmp(item.container_name, 'CS2') == 1
            tmp=[double(item.call_time), 0.2];
            annotations = cat(1, annotations, {tmp(1), tmp(2), strcat(item.container_name,'.',item.call_name,' Write')});
            write_CS2 = cat(1, write_CS2, tmp);
        end
    end
end

% get the maximum time
mM = max(updateContainer_CS1(:,1));
mS = max(updateContainer_S(:,1));
mR = max(updateContainer_R(:,1));
mM2 = max(updateContainer_CS2(:,1));
mS2 = max(updateContainer_SSS(:,1));
mMaxTime = max(mS2, max(mM2, max(mM, max(mS, mR))));

figure
% ax2 = subplot(5,1,1);
% hold on
% plot(ax2, updateContainer_S(:,1), updateContainer_S(:,2), '-.og')
% sSize = size(write_S);
% if (sSize(1)) > 0
% plot(ax2, write_S(:,1), write_S(:,2), '-wo',...
%                 'LineWidth',0.001,...
%                 'MarkerEdgeColor','k',...
%                 'MarkerSize',5)
% end
% hold off
% title(ax2,'Execution of S')

% ax3 = subplot(5,1,2);
% hold on
% plot(ax3, updateContainer_R(:,1), updateContainer_R(:,2), '-.ob')
% rnewSize = size(read_new_R);
% if (rnewSize(1)) > 0
% plot(ax3, read_new_R(:,1), read_new_R(:,2), '-wo',...
%                 'LineWidth',0.001,...
%                 'MarkerEdgeColor','m',...
%                 'MarkerSize',5)
% end
% old_size = size(read_old_R);
% old_size = old_size(1);
% if old_size > 0
% plot(ax3, read_old_R(:,1), read_old_R(:,2), '-wo',...
%                 'LineWidth',0.001,...
%                 'MarkerEdgeColor','r',...
%                 'MarkerSize',5)
% end
% hold off
% title(ax3,'Execution of R')
% %ylabel(ax3,'Values from 0 to 1')

ax1 = subplot(2,1,1);
hold on
plot(ax1, updateContainer_CS1(:,1), updateContainer_CS1(:,2), '-or')
wcs1Size = size(write_CS1);
if (wcs1Size(1)) > 0
plot(ax1, write_CS1(:,1), write_CS1(:,2), '-wo',...
                'LineWidth',0.001,...
                'MarkerEdgeColor','k',...
                'MarkerSize',5)
end
plot(ax1, updateContainer_S(:,1), updateContainer_S(:,2), '-.og')
hatch_S_size = size(hatch_S_X);
for i = 1:hatch_S_size(1)
    h = patch(hatch_S_X(i,:), hatch_S_Y(i,:), 'green');
    set(h,'edgecolor','green');
    hatchfill(h, 'single', 45, 5, 'none', 'g', 0.3);
end
sSize = size(write_S);
if (sSize(1)) > 0
plot(ax1, write_S(:,1), write_S(:,2), '-wo',...
                'LineWidth',0.001,...
                'MarkerEdgeColor','k',...
                'MarkerSize',5)
end
plot(ax1, updateContainer_R(:,1), updateContainer_R(:,2), '-.ob')
hatch_R_size = size(hatch_R_X);
for i = 1:hatch_R_size(1)
    h = patch(hatch_R_X(i,:), hatch_R_Y(i,:), 'blue');
    set(h,'edgecolor','blue');
    hatchfill(h, 'single', 45, 5, 'none', 'b', 0.3);
end
rnewSize = size(read_new_R);
if (rnewSize(1)) > 0
plot(ax3, read_new_R(:,1), read_new_R(:,2), '-wo',...
                'LineWidth',0.001,...
                'MarkerEdgeColor','m',...
                'MarkerSize',5)
end
old_size = size(read_old_R);
old_size = old_size(1);
if old_size > 0
plot(ax1, read_old_R(:,1), read_old_R(:,2), '-wo',...
                'LineWidth',0.001,...
                'MarkerEdgeColor','r',...
                'MarkerSize',5)
end

% text(annotations{:,1},annotations{:,2},annotations{:,3},'HorizontalAlignment','right')


hold off
title(ax1,'Execution of CS1')
%ylabel(ax1,'Values from -1 to 1')

ax4 = subplot(2,1,2);
hold on
plot(ax4, updateContainer_CS2(:,1), updateContainer_CS2(:,2), '-or')
wcs2Size = size(write_CS2);
if (wcs2Size(1)) > 0
plot(ax4, write_CS2(:,1), write_CS2(:,2), '-wo',...
                'LineWidth',0.001,...
                'MarkerEdgeColor','k',...
                'MarkerSize',5)
end
% area(ax4, updateContainer_SSS(:,1), updateContainer_SSS(:,2), 'FaceColor','g','EdgeColor','none')
plot(ax4, updateContainer_SSS(:,1), updateContainer_SSS(:,2), '-.og')
plot(ax4, write_SSS(:,1), write_SSS(:,2), '-wo',...
                'LineWidth',0.001,...
                'MarkerEdgeColor','k',...
                'MarkerSize',5)

hatch_SSS_size = size(hatch_SSS_X);
for i = 1:hatch_SSS_size(1)
    h = patch(hatch_SSS_X(i,:), hatch_SSS_Y(i,:), 'green');
    set(h,'edgecolor','green');
    hatchfill(h, 'single', 45, 5, 'none', 'g', 0.3);
end
hold off
title(ax4,'Execution of CS2')

% ax5 = subplot(3,1,3);
% hold on
% plot(ax5, updateContainer_SSS(:,1), updateContainer_SSS(:,2), '-.og')
% plot(ax5, write_SSS(:,1), write_SSS(:,2), '-wo',...
%                 'LineWidth',0.001,...
%                 'MarkerEdgeColor','k',...
%                 'MarkerSize',5)
% hold off
% title(ax5,'Execution of S2')

% linkaxes([ax1,ax2,ax3,ax4,ax5],'x')
ylim([0 1])
linkaxes([ax1,ax4],'x')
pbaspect
