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
  + geom_text(aes(label = V2, vjust = "outward", hjust = "outward"))
  + xlab("Functions (1 = t_loadImages, 2 = t_calculateMedianHues, 3 = t_sortImagesByHue)")
  + ylab("Time (ms)")