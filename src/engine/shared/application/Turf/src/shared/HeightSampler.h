#ifndef INCLUDED_HeightSampler_H
#define INCLUDED_HeightSampler_H

class Rectangle2d;

class HeightSampler
{
public:
	enum TERRAIN_CHANNEL
	{
		CH_HEIGHT                         =(1<<0),
		CH_COLOR                          =(1<<1),
		CH_SHADER                         =(1<<2),
		CH_FLORA_STATIC_COLLIDABLE        =(1<<3),
		CH_FLORA_STATIC_NON_COLLIDABLE    =(1<<4),
		CH_FLORA_DYNAMIC_NEAR             =(1<<5),
		CH_FLORA_DYNAMIC_FAR              =(1<<6),
		CH_ENVIRONMENT                    =(1<<7),
		CH_VERTEX_POSITION                =(1<<8),
		CH_VERTEX_NORMAL                  =(1<<9),
		CH_EXCLUDE                        =(1<<10),
		CH_PASSABLE                       =(1<<11)
	};

	static bool sample(const char *i_terrain_file, const char *i_output_file, TERRAIN_CHANNEL i_channel);
	static bool compareSamples(const char *i_original_sample_file, const char *i_new_sample_file);
	static bool writeTGA(
		const char *o_tga_base_path, 
		const char *i_sample_file1, 
		const char *i_diff_file=0
		);

	static bool createStaticFloraList(const char *i_terrain_file, const char *o_flora_file, const Rectangle2d &clipRectangle);

};

#endif
