#pragma once

//Code by LHD, 2017.03.18. A homework of Computer Vision
//Base on CImg

#ifndef COLOR_TRANSFER
#define COLOR_TRANSFER

#include <map>
#include "CImg.h"

using namespace std;
using namespace cimg_library;

class ColorTransfer {
  public:
    // Run the approach of the paper
    static CImg<float> Run(CImg<unsigned char> target, CImg<unsigned char> source) {
        CImg<float> target_lab = target.get_RGBtoLab();
        CImg<float> source_lab = source.get_RGBtoLab();
        CImg<float> ans_lab(source_lab);
        CImg<float> new_l_channel = ColorTransfer::CombineChannel(target_lab.get_channel(0), source_lab.get_channel(0));
        CImg<float> new_a_channel = ColorTransfer::CombineChannel(target_lab.get_channel(1), source_lab.get_channel(1));
        CImg<float> new_b_channel = ColorTransfer::CombineChannel(target_lab.get_channel(2), source_lab.get_channel(2));
        cimg_forXY(ans_lab, x, y) {
            ans_lab(x, y, 0) = new_l_channel(x, y);
            ans_lab(x, y, 1) = new_a_channel(x, y);
            ans_lab(x, y, 2) = new_b_channel(x, y);
        }
        CImg<float> ans = ans_lab.get_LabtoRGB();
        return ans;
    }
    
    /*
    Combine the value of two channels according to the paper
    */
    static CImg<float> CombineChannel(CImg<float> target, CImg<float> source) {
        
        double t_mean = target.mean(), s_mean = source.mean();
        double t_var = target.variance(), s_var = source.variance();
        CImg<float> ans(source);
        cimg_forXY(ans, x, y) {
            ans(x, y) = t_var / s_var * (source(x, y) - s_mean) + t_mean;
        }
        return ans;
    }
    
    //A sample function to show the histogram of a picture
    template <typename T>
    static void ShowHistogram(CImg<T> img, int v, int min, int max, char * filepath = "") {
        CImg<float> his = img.get_histogram(v, min, max);
        if (filepath != "") 
        his.display_graph().save_bmp(filepath);
    }
    
    // show the histogram
    // useless
    static CImg<float> GetHis(CImg<float> img, int c, int min, int max) {
        int v = (max - min + 1)*c;
        CImg<float> ans(v, 1, 1, 1, 0);
        cimg_forXY(img, x, y) {
            float t = img(x, y);
            int val = (int)roundf((img(x, y) - min) * c);
            ans(val) += 1.0;
        }
        return ans;
    }
    
    //NewCC means NewCombineChannels.
    //A new approach to the paper, modify by me.
    //I use the hisogram equalization to modify the job.
    static CImg<float> NewCC(CImg<float> target, CImg<float> source, int min, int max, int c) {
        CImg<float> ans(source);
        int v = (max - min + 1)*c;
        CImg<float> t_his = ColorTransfer::GetHis(target, c, min, max);
        CImg<float> s_his = ColorTransfer::GetHis(source, c, min, max);
        CImg<float> t_cdf(v, 1, 1, 1), s_cdf(v, 1, 1, 1);
        float t_sum = 0.0, s_sum = 0.0;
        float t_size = target._width * target._height;
        float s_size = source._width * source._height;
        cimg_forX(t_his, x) {
            t_sum += t_his(x);
            s_sum += s_his(x);
            t_cdf(x) = t_sum;
            s_cdf(x) = s_sum;
        }
        cimg_forX(s_his, x) {
            t_cdf(x) /= t_sum;
            s_cdf(x) /= s_sum;
        }
        map<int, int> T;
        for (int i = 0, j = 0; i < v; ) {
            if (s_cdf(i) <= t_cdf(j)) {
                T[i] = j;
                i++;
            } else {
                j++;
            }
        }
        cimg_forXY(ans, x, y) {
            float temp = (float)(T[(int)((source(x, y) - min)*c)]) / c + min;
            ans(x, y) = temp;
        }
        return ans;
    }

    //Run the new approach
    static CImg<float> Run_New(CImg<unsigned char> target, CImg<unsigned char> source, int c) {
        CImg<float> target_lab = target.get_RGBtoLab();
        CImg<float> source_lab = source.get_RGBtoLab();
        CImg<float> ans_lab(source_lab);
        CImg<float> new_l_channel;
        new_l_channel = ColorTransfer::CombineChannel(target_lab.get_channel(0), source_lab.get_channel(0));
        CImg<float> new_a_channel = ColorTransfer::NewCC(target_lab.get_channel(1), source_lab.get_channel(1), -128, 127, c);
        CImg<float> new_b_channel = ColorTransfer::NewCC(target_lab.get_channel(2), source_lab.get_channel(2), -128, 127, c);
        cimg_forXY(ans_lab, x, y) {
            ans_lab(x, y, 0) = new_l_channel(x, y);
            ans_lab(x, y, 1) = new_a_channel(x, y);
            ans_lab(x, y, 2) = new_b_channel(x, y);
        }
        CImg<float> ans = ans_lab.get_LabtoRGB();
        ans.sharpen(50.0f);
        return ans;
    }
};

#endif // !COLOR_TRANSFER
