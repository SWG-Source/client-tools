#ifndef __UISYSTEMDEPENDANCIES_H__
#define __UISYSTEMDEPENDANCIES_H__

class UISystemDependancies
{
public:

			 UISystemDependancies();
			~UISystemDependancies();

	long GetTickCount( void ) const ;
	long GetCaratBlinkTickCount( void ) const;

	static UISystemDependancies &Get( void );
};

#endif // __UISYSTEMDEPENDANCIES_H__
