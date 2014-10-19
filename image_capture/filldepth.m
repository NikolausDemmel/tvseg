dataset = dir('new/*_depth.png');

for i=1:size(dataset,1)
    depthfile = strcat('new/',dataset(i).name);
    colorfile = strrep(depthfile, 'depth', 'color');
  
    depth = double(imread(depthfile))/10000.0;
    color = imread(colorfile);
    color_double = double(color)/255.0;
    
    filled1 = fill_depth_colorization(color_double, depth);
    filled2 = fill_depth_cross_bf(color, depth);
    
    filled1=uint16(filled1*10000.0);
    filled2=uint16(filled2*10000.0);
    
    imwrite(filled1, strcat(strrep(depthfile, 'depth', 'depth_filled_colorization')));
    imwrite(filled2, strcat(strrep(depthfile, 'depth', 'depth_filled_cross_bf')));
end
