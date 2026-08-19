/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/


void GLLandscapeViewer::init()
{
    wheelX->setOrientation( Horizontal );
    wheelY->setOrientation( Horizontal );
    wheelZ->setOrientation( Horizontal );
    
    wheelZoom->setOrientation( Vertical );
    wheelZoom->setRange( 0, 200 );  
    wheelZoom->setValue( 100 );

    wheelSize->setOrientation( Vertical ); 
    wheelSize->setRange( 25, 200 );   
    wheelSize->setValue( 50 ); 
    wheelSize->setSteps( 10, 10 );
   	
    wheelX->setRange( 0, 360 ); 
    wheelY->setRange( 0, 360 );  
    wheelZ->setRange( 0, 360 );  
	
    wheelX->setTransmissionRatio( 0.5 ); 
    wheelY->setTransmissionRatio( 0.5 );  
    wheelZ->setTransmissionRatio( 0.5 );  
}

void GLLandscapeViewer::destroy()
{

}
