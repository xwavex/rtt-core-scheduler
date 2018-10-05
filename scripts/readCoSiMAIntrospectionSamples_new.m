clear;
fname = "/home/dwigand/code/cogimon/CoSimA/component-repositories/coman-whole-body/components/lib/orocos/saveNewPaper3/rtReport3.dat"; 
fid = fopen(fname); 
raw = fread(fid,inf); 
str = char(raw'); 
fclose(fid); 
val = jsondecode(str);

amountOfData = size(val.root);
amountOfData = amountOfData(1);

updateContainer_CS3 = [];
updateContainer_COM = [];
updateContainer_BASE = [];
updateContainer_RGZ = [];
updateContainer_IK = [];

read_new_R = [];
read_old_R = [];
write_SSS = [];
write_S = [];
write_CS1 = [];
write_CS2 = [];

write_RGZ_DOT = [];

hatch_IK_X = [];
hatch_IK_Y = [];
hatch_COM_X = [];
hatch_COM_Y = [];
hatch_BASE_X = [];
hatch_BASE_Y = [];

annotations = {};

for index = 1:amountOfData
    item = val.root(index);
    if strcmp(item.call_name, 'updateHook()') == 1
        if strcmp(item.container_name, 'CS_3_a1e') == 1
             %disp(item)
            tmp=[double(item.call_time), 0; double(item.call_time), 1; double(item.call_duration), 1; double(item.call_duration), 0];
            updateContainer_CS3 = cat(1, updateContainer_CS3, tmp);
        elseif strcmp(item.container_name, 'com') == 1
            tmp=[double(item.call_time), 0; double(item.call_time), 0.8; double(item.call_duration), 0.8; double(item.call_duration), 0];
            hatch_COM_X = cat(1, hatch_COM_X, [double(item.call_time), double(item.call_time), double(item.call_duration), double(item.call_duration)]);
            hatch_COM_Y = cat(1, hatch_COM_Y, [0,0.8,0.8,0]);
            updateContainer_COM = cat(1, updateContainer_COM, tmp);
        elseif strcmp(item.container_name, 'base') == 1
            tmp=[double(item.call_time), 0; double(item.call_time), 0.8; double(item.call_duration), 0.8; double(item.call_duration), 0];
            hatch_BASE_X = cat(1, hatch_BASE_X, [double(item.call_time), double(item.call_time), double(item.call_duration), double(item.call_duration)]);
            hatch_BASE_Y = cat(1, hatch_BASE_Y, [0,0.8,0.8,0]);
            updateContainer_BASE = cat(1, updateContainer_BASE, tmp);
        elseif strcmp(item.container_name, 'ik') == 1
            tmp=[double(item.call_time), 0; double(item.call_time), 0.8; double(item.call_duration), 0.8; double(item.call_duration), 0];
            hatch_IK_X = cat(1, hatch_IK_X, [double(item.call_time), double(item.call_time), double(item.call_duration), double(item.call_duration)]);
            hatch_IK_Y = cat(1, hatch_IK_Y, [0,0.8,0.8,0]);
            updateContainer_IK = cat(1, updateContainer_IK, tmp);
%         elseif strcmp(item.container_name, 'CS_1_a1e') == 1
%             tmp=[double(item.call_time), 0; double(item.call_time), 1; double(item.call_duration), 1; double(item.call_duration), 0];
%             updateContainer_CS0 = cat(1, updateContainer_CS0, tmp);
        end
    elseif strcmp(item.call_name, 'WorldUpdate()') == 1
        if strcmp(item.container_name, 'robot_gazebo') == 1
            tmp=[double(item.call_time), 0; double(item.call_time), 0.8; double(item.call_duration), 0.8; double(item.call_duration), 0];
            updateContainer_RGZ = cat(1, updateContainer_RGZ, tmp);
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
        elseif strcmp(item.container_name, 'robot_gazebo') == 1
            if strcmp(item.call_name, 'out_signal_dot_finished_port') == 1
                tmp=[double(item.call_time), 0.6];
                annotations = cat(1, annotations, {tmp(1), tmp(2), strcat(item.container_name,'.',item.call_name,' Write')});
                write_RGZ_DOT = cat(1, write_RGZ_DOT, tmp);    
            end
        end
    end
end

% get the maximum time
mM = max(updateContainer_CS3(:,1));
mS = max(updateContainer_COM(:,1));
mR = max(updateContainer_BASE(:,1));
mM2 = max(updateContainer_RGZ(:,1));
mS2 = max(updateContainer_IK(:,1));
mMaxTime = max(mS2, max(mM2, max(mM, max(mS, mR))));

figure
ax1 = subplot(2,1,1);
hold on
plot(ax1, updateContainer_CS3(:,1), updateContainer_CS3(:,2), '-or')
wcs1Size = size(write_CS1);
if (wcs1Size(1)) > 0
plot(ax1, write_CS1(:,1), write_CS1(:,2), '-wo',...
                'LineWidth',0.001,...
                'MarkerEdgeColor','k',...
                'MarkerSize',5)
end
plot(ax1, updateContainer_COM(:,1), updateContainer_COM(:,2), '-.og')
hatch_S_size = size(hatch_COM_X);
for i = 1:hatch_S_size(1)
    h = patch(hatch_COM_X(i,:), hatch_COM_Y(i,:), 'green');
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
plot(ax1, updateContainer_BASE(:,1), updateContainer_BASE(:,2), '-.ob')
hatch_R_size = size(hatch_BASE_X);
for i = 1:hatch_R_size(1)
    h = patch(hatch_BASE_X(i,:), hatch_BASE_Y(i,:), 'blue');
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

plot(ax1, updateContainer_IK(:,1), updateContainer_IK(:,2), '-.og')
% plot(ax4, write_SSS(:,1), write_SSS(:,2), '-wo',...
%                 'LineWidth',0.001,...
%                 'MarkerEdgeColor','k',...
%                 'MarkerSize',5)

hatch_SSS_size = size(hatch_IK_X);
for i = 1:hatch_SSS_size(1)
    h = patch(hatch_IK_X(i,:), hatch_IK_Y(i,:), 'green');
    set(h,'edgecolor','green');
    hatchfill(h, 'single', 45, 5, 'none', 'g', 0.3);
end

hold off
title(ax1,'Execution of CS1')
%ylabel(ax1,'Values from -1 to 1')

ax4 = subplot(2,1,2);
hold on
plot(ax4, updateContainer_RGZ(:,1), updateContainer_RGZ(:,2), '-or')
wrgzdotSize = size(write_RGZ_DOT);
if (wrgzdotSize(1)) > 0
plot(ax4, write_RGZ_DOT(:,1), write_RGZ_DOT(:,2), '-wo',...
                'LineWidth',0.001,...
                'MarkerEdgeColor','k',...
                'MarkerSize',5)
end

wcs2Size = size(write_CS2);
if (wcs2Size(1)) > 0
plot(ax4, write_CS2(:,1), write_CS2(:,2), '-wo',...
                'LineWidth',0.001,...
                'MarkerEdgeColor','k',...
                'MarkerSize',5)
end
hold off
title(ax4,'Execution of CS2')

% ax5 = subplot(3,1,3);
% hold on
% plot(ax5, updateContainer_IK(:,1), updateContainer_IK(:,2), '-.og')
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
