# AmongusFinder5000
A script that can search an image for a template

Originally this was intended to find occurences of Amongus within [r/place](https://www.reddit.com/r/place). Now it can find every occurence of any template in a PNG image.

## How it works
The script takes two images "source.png" and "template.png". Source.png is the image to search for template.png.

The template is formatted as such:
- Each pixel with 255 Red will be ignored in the matching
- Each pixel with 1-254 Red must be the same color
- Each pixel with 0 Red must be different from the main color

It will return two images "results.png" and "catalogue.png" Results.png contains marked matches in the original image. Catalogue.png contains a copy of every match in a table.

To match the template, it is slid across every pixel of the source and evaluated at every pixel position. Therefore this can be a somewhat costly operation if working with multiple very large templates or sources.

Thanks to Lode Vandevenne for [LodePNG](https://github.com/lvandeve/lodepng)