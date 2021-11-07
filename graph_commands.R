# load ggplot2
library(ggplot2)

# command to get a graph of the function times
# in the CSV file, the function names should be changed to:
#   t_loadImages = 1
#   t_calculateMedianHues = 2
#   t_sortImagesByHue = 3
# this is so that the graph can be delivered as a line graph as worded (discrete) data cannot be
ggplot(data=times, aes(x=V1, y=V2))
  + geom_line()
  + geom_text(aes(label=V2, vjust="outward", hjust="outward"))
  + xlab("Parallel Functions (1 = t_loadImages, 2 = t_calculateMedianHues, 3 = t_sortImagesByHue)")
  + ylab("Time (ms)")

# for the sequential function
# the only difference is the x-axis label as the sequential function isn't split into separate functions
# so the three major operations can't be identified by their function name
ggplot(data=times, aes(x=V1, y=V2))
  + geom_line()
  + geom_text(aes(label=V2, vjust="outward", hjust="outward"))
  + xlab("Sequential Function stages (1 = Load Images, 2 = Calculate Median Hues, 3 = Sort Images by Hue)")
  + ylab("Time (ms)")