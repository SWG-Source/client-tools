/* Copyright (c) 2000, 2001, Oracle Corporation.  All rights reserved.  */
 
/* 
   NAME 
     occiCommon.h - header file for doing forward references

   DESCRIPTION 
     Just declare all the classes

   RELATED DOCUMENTS 
     <note any documents related to this facility>
 
   EXPORT FUNCTION(S) 
     <external functions declared for use outside package - one-line descriptions>

   INTERNAL FUNCTION(S)
     <other external functions declared - one-line descriptions>

   EXAMPLES

   NOTES
     <other useful comments, qualifications, etc.>

   MODIFIED   (MM/DD/YY)
   rvallam     07/24/01 - fix bug 1777042 - removed namespace definition
                          for HPUX
   rvallam     06/11/01 - fixed NT porting problem with template functions-
                          added generic method get(set)VectorOfRefs
   rratnam     04/25/01 - fixed NT porting bug1673780
   rvallam     04/12/01 - added dummy parameter in getVector of AnyData
                          for PObject *
   rratnam     04/10/01 - removed references to wstring
   rvallam     04/02/01 - fixed linux porting bug 1654670
   gayyappa    03/29/01 - remove get/set vector methods for int/float/double/
                          unsigned int for anydata.
   rvallam     03/20/01 - added dummy parameter for Type in getVector for
                          void * in statement and ResultSet
                          added setVector of Ref<T> and Number prototype
                          for statement
   gayyappa    03/15/01 - added parameter to getvector on anydata with void*.
   rratnam     03/13/01 - added virtual destructor to RefCounted
   rkasamse    03/15/01 - add an arg, Type, to getVector of void*
   rratnam     03/06/01 - removed references() from RefCounted
   slari       02/15/01 - suppress lint warnings
   rratnam     02/12/01 - removed #include <iostream.h>
   rvallam     02/07/01 - added enum OCCI_MAX_PREFETCH_DEPTH
   gayyappa    01/15/01 - change ub4 to unsigned int for
                          get/set vector methods..
   gayyappa    01/03/01 - put "using namespace std" in a ifndef
   gayyappa    12/14/00 - add forward declaration for ResultSetImpl 
                          and StatementImpl.
                          Remove commented #defines
   gayyappa    11/17/00 - change Session to Connection
   gayyappa    09/12/00 - remove OCCI_SQLT_INT, OCCI_SQLT_FLT 
                          remove OCCIUNSIGNED_CHAR, OCCISHORT, 
                          OCCIUNSIGNED_SHORT, OCCILONG ,OCCI_LONGDOUBLE,
                          OCCI_UNSIGNEDLONG  from enum Type.
   gayyappa    08/25/00 - add const to get/set vector mthds of anydata.
   rvallam     08/10/00 - updated Type enum
   slari       08/04/00 - add OCCIROWID and OCCICURSOR
   rkasamse    08/04/00 - add setVector methods
   slari       07/24/00 - add BytesImpl
   rratnam     07/25/00 - Added forward declarations for LobStreamImpl,
                          ConnectionPool[Impl], removed those for
                          Connection[Impl]
   rkasamse    07/28/00 - add getVector(ResultSet*...) methods
   rratnam     06/14/00 - Added forward declaration for RefImpl,
                          added DefaultCharSet to the CharSet 
                          enum
   rratnam     06/13/00 - Added DefaultCharSet to the CharSet enum
   kmohan      05/31/00 - RefCounted no more templated
   rratnam     22/05/00 - Added forward declaration of ConnectionImpl, and the
                         LobOpenMode enum
   kmohan      05/05/00 - Added global routine prototypes
   rkasamse    05/23/00 -
   slari       04/28/00 - add forward declaration of SQLExceptionImpl
   kmohan      04/19/00 - Added enums Type,CharSet,CharSetForm
   gayyappa    04/18/00 - removed checkStatus. added checkOCICall and 
                          createSQLEXception functions 
   kmohan      04/11/00 - Creation

*/


#ifndef OCCICOMMON_ORACLE
# define OCCICOMMON_ORACLE

#ifndef _olint

#ifndef OCI_ORACLE
#include <oci.h>
#endif

#ifndef ORASTRING
#define ORASTRING
#include <string>
#endif

#ifndef ORAVECTORSTL
#include <vector>
#define ORAVECTORSTL
#endif

#ifndef ORALISTSTL
#include <list>
#define ORALISTSTL
#endif

#define OCCI_STD_NAMESPACE std
#define OCCI_HAVE_STD_NAMESPACE 1

namespace oracle {
namespace occi {
class Environment;
class EnvironmentImpl;
class Connection;
class ConnectionImpl;
class ConnectionPool;
class ConnectionPoolImpl;
class Statement;
class StatementImpl;
class ResultSet;
class ResultSetImpl;
class SQLException;
class SQLExceptionImpl;
class Stream;
class PObject;
class Number;
class Bytes;
class BytesImpl;
class Date;
class Timestamp;

class MetaData;
class MetaDataImpl;
template <class T> class Ref;
class RefImpl;
class RefAny;
class Blob;
class Bfile;
class Clob;
class LobStreamImpl;
class AnyData;
class AnyDataImpl;
class Map;
class IntervalDS;
class IntervalYM;


/*---------------------------------------------------------------------------
                           ENUMERATORS
  ---------------------------------------------------------------------------*/
enum Type
{
 OCCI_SQLT_CHR=SQLT_CHR,
 OCCI_SQLT_NUM=SQLT_NUM,
 OCCIINT = SQLT_INT,
 OCCIFLOAT = SQLT_FLT,
 OCCI_SQLT_STR=SQLT_STR,
 OCCI_SQLT_VNU=SQLT_VNU,
 OCCI_SQLT_PDN=SQLT_PDN,
 OCCI_SQLT_LNG=SQLT_LNG,
 OCCI_SQLT_VCS=SQLT_VCS,
 OCCI_SQLT_NON=SQLT_NON,
 OCCI_SQLT_RID=SQLT_RID,
 OCCI_SQLT_DAT=SQLT_DAT,
 OCCI_SQLT_VBI=SQLT_VBI,
 OCCI_SQLT_BIN=SQLT_BIN,
 OCCI_SQLT_LBI=SQLT_LBI,
 OCCIUNSIGNED_INT = SQLT_UIN,
 OCCI_SQLT_SLS=SQLT_SLS,
 OCCI_SQLT_LVC=SQLT_LVC,
 OCCI_SQLT_LVB=SQLT_LVB,
 OCCI_SQLT_AFC=SQLT_AFC,
 OCCI_SQLT_AVC=SQLT_AVC,
 OCCI_SQLT_CUR=SQLT_CUR,
 OCCI_SQLT_RDD=SQLT_RDD,
 OCCI_SQLT_LAB=SQLT_LAB,
 OCCI_SQLT_OSL=SQLT_OSL,
 OCCI_SQLT_NTY=SQLT_NTY,
 OCCI_SQLT_REF=SQLT_REF,
 OCCI_SQLT_CLOB=SQLT_CLOB,
 OCCI_SQLT_BLOB=SQLT_BLOB,
 OCCI_SQLT_BFILEE=SQLT_BFILEE,
 OCCI_SQLT_CFILEE=SQLT_CFILEE,
 OCCI_SQLT_RSET=SQLT_RSET,
 OCCI_SQLT_NCO=SQLT_NCO,
 OCCI_SQLT_VST=SQLT_VST,
 OCCI_SQLT_ODT=SQLT_ODT,
 OCCI_SQLT_DATE=SQLT_DATE,
 OCCI_SQLT_TIME=SQLT_TIME,
 OCCI_SQLT_TIME_TZ=SQLT_TIME_TZ,
 OCCI_SQLT_TIMESTAMP=SQLT_TIMESTAMP,
 OCCI_SQLT_TIMESTAMP_TZ=SQLT_TIMESTAMP_TZ,
 OCCI_SQLT_INTERVAL_YM=SQLT_INTERVAL_YM,
 OCCI_SQLT_INTERVAL_DS=SQLT_INTERVAL_DS,
 OCCI_SQLT_TIMESTAMP_LTZ=SQLT_TIMESTAMP_LTZ,
 OCCI_SQLT_FILE=SQLT_FILE,
 OCCI_SQLT_CFILE=SQLT_CFILE,
 OCCI_SQLT_BFILE=SQLT_BFILE,
 
 OCCICHAR = 32 *1024,
 OCCIDOUBLE,
 OCCIBOOL,
 OCCIANYDATA ,
 OCCINUMBER,
 OCCIBLOB,
 OCCIBFILE,
 OCCIBYTES,
 OCCICLOB ,
 OCCIVECTOR,
 OCCIMETADATA,
 OCCIPOBJECT,
 OCCIREF ,
 OCCIREFANY,
 OCCISTRING  ,
 OCCISTREAM  ,
 OCCIDATE  ,
 OCCIINTERVALDS  ,
 OCCIINTERVALYM  ,
 OCCITIMESTAMP,
 OCCIROWID,
 OCCICURSOR


};

enum LockOptions {OCCI_LOCK_NONE = OCI_LOCK_NONE,
                      OCCI_LOCK_X = OCI_LOCK_X, 
                      OCCI_LOCK_X_NOWAIT = OCI_LOCK_X_NOWAIT
                     };

enum {OCCI_MAX_PREFETCH_DEPTH = UB4MAXVAL};

enum TypeCode
{

OCCI_TYPECODE_REF = OCI_TYPECODE_REF,
OCCI_TYPECODE_DATE = OCI_TYPECODE_DATE,
OCCI_TYPECODE_REAL = OCI_TYPECODE_REAL,
OCCI_TYPECODE_DOUBLE = OCI_TYPECODE_DOUBLE,
OCCI_TYPECODE_FLOAT = OCI_TYPECODE_FLOAT,
OCCI_TYPECODE_NUMBER = OCI_TYPECODE_NUMBER,
OCCI_TYPECODE_DECIMAL = OCI_TYPECODE_DECIMAL,
OCCI_TYPECODE_OCTET = OCI_TYPECODE_OCTET,
OCCI_TYPECODE_INTEGER = OCI_TYPECODE_INTEGER,
OCCI_TYPECODE_SMALLINT= OCI_TYPECODE_SMALLINT,
OCCI_TYPECODE_RAW = OCI_TYPECODE_RAW,
OCCI_TYPECODE_VARCHAR2 = OCI_TYPECODE_VARCHAR2,
OCCI_TYPECODE_VARCHAR = OCI_TYPECODE_VARCHAR,
OCCI_TYPECODE_CHAR = OCI_TYPECODE_CHAR,
OCCI_TYPECODE_VARRAY= OCI_TYPECODE_VARRAY,
OCCI_TYPECODE_TABLE = OCI_TYPECODE_TABLE,
OCCI_TYPECODE_CLOB = OCI_TYPECODE_CLOB,
OCCI_TYPECODE_BLOB = OCI_TYPECODE_BLOB,
OCCI_TYPECODE_BFILE = OCI_TYPECODE_BFILE,
OCCI_TYPECODE_OBJECT = OCI_TYPECODE_OBJECT,
OCCI_TYPECODE_NAMEDCOLLECTION = OCI_TYPECODE_NAMEDCOLLECTION
};


enum CharSet	
{
    DefaultCharSet =0
   ,US7ASCII =1
   ,WE8DEC =2
   ,WE8HP =3
   ,US8PC437 =4
   ,WE8EBCDIC37 =5
   ,WE8EBCDIC500 =6
   ,WE8EBCDIC1140 =7
   ,WE8EBCDIC285 =8
   ,WE8EBCDIC1146 =9
   ,WE8PC850 =10
   ,D7DEC =11
   ,F7DEC =12
   ,S7DEC =13
   ,E7DEC =14
   ,SF7ASCII =15
   ,NDK7DEC =16
   ,I7DEC =17
   ,NL7DEC =18
   ,CH7DEC =19
   ,YUG7ASCII =20
   ,SF7DEC =21
   ,TR7DEC =22
   ,IW7IS960 =23
   ,IN8ISCII =25
   ,WE8EBCDIC1148 =27
   ,WE8PC858 =28
   ,WE8ISO8859P1 =31
   ,EE8ISO8859P2 =32
   ,SE8ISO8859P3 =33
   ,NEE8ISO8859P4 =34
   ,CL8ISO8859P5 =35
   ,AR8ISO8859P6 =36
   ,EL8ISO8859P7 =37
   ,IW8ISO8859P8 =38
   ,WE8ISO8859P9 =39
   ,NE8ISO8859P10 =40
   ,TH8TISASCII =41
   ,TH8TISEBCDIC =42
   ,BN8BSCII =43
   ,VN8VN3 =44
   ,VN8MSWIN1258 =45
   ,WE8ISO8859P15 =46
   ,WE8NEXTSTEP =50
   ,AR8ASMO708PLUS =61
   ,AR8EBCDICX =70
   ,AR8XBASIC =72
   ,EL8DEC =81
   ,TR8DEC =82
   ,WE8EBCDIC37C =90
   ,WE8EBCDIC500C =91
   ,IW8EBCDIC424 =92
   ,TR8EBCDIC1026 =93
   ,WE8EBCDIC871 =94
   ,WE8EBCDIC284 =95
   ,WE8EBCDIC1047 =96
   ,WE8EBCDIC1140C =97
   ,WE8EBCDIC1145 =98
   ,WE8EBCDIC1148C =99
   ,EEC8EUROASCI =110
   ,EEC8EUROPA3 =113
   ,LA8PASSPORT =114
   ,BG8PC437S =140
   ,EE8PC852 =150
   ,RU8PC866 =152
   ,RU8BESTA =153
   ,IW8PC1507 =154
   ,RU8PC855 =155
   ,TR8PC857 =156
   ,CL8MACCYRILLIC =158
   ,CL8MACCYRILLICS =159
   ,WE8PC860 =160
   ,IS8PC861 =161
   ,EE8MACCES =162
   ,EE8MACCROATIANS =163
   ,TR8MACTURKISHS =164
   ,IS8MACICELANDICS =165
   ,EL8MACGREEKS =166
   ,IW8MACHEBREWS =167
   ,EE8MSWIN1250 =170
   ,CL8MSWIN1251 =171
   ,ET8MSWIN923 =172
   ,BG8MSWIN =173
   ,EL8MSWIN1253 =174
   ,IW8MSWIN1255 =175
   ,LT8MSWIN921 =176
   ,TR8MSWIN1254 =177
   ,WE8MSWIN1252 =178
   ,BLT8MSWIN1257 =179
   ,D8EBCDIC273 =180
   ,I8EBCDIC280 =181
   ,DK8EBCDIC277 =182
   ,S8EBCDIC278 =183
   ,EE8EBCDIC870 =184
   ,CL8EBCDIC1025 =185
   ,F8EBCDIC297 =186
   ,IW8EBCDIC1086 =187
   ,CL8EBCDIC1025X =188
   ,D8EBCDIC1141 =189
   ,N8PC865 =190
   ,BLT8CP921 =191
   ,LV8PC1117 =192
   ,LV8PC8LR =193
   ,BLT8EBCDIC1112 =194
   ,LV8RST104090 =195
   ,CL8KOI8R =196
   ,BLT8PC775 =197
   ,DK8EBCDIC1142 =198
   ,S8EBCDIC1143 =199
   ,I8EBCDIC1144 =200
   ,F7SIEMENS9780X =201
   ,E7SIEMENS9780X =202
   ,S7SIEMENS9780X =203
   ,DK7SIEMENS9780X =204
   ,N7SIEMENS9780X =205
   ,I7SIEMENS9780X =206
   ,D7SIEMENS9780X =207
   ,F8EBCDIC1147 =208
   ,WE8GCOS7 =210
   ,EL8GCOS7 =211
   ,US8BS2000 =221
   ,D8BS2000 =222
   ,F8BS2000 =223
   ,E8BS2000 =224
   ,DK8BS2000 =225
   ,S8BS2000 =226
   ,WE8BS2000 =231
   ,CL8BS2000 =235
   ,WE8BS2000L5 =239
   ,WE8DG =241
   ,WE8NCR4970 =251
   ,WE8ROMAN8 =261
   ,EE8MACCE =262
   ,EE8MACCROATIAN =263
   ,TR8MACTURKISH =264
   ,IS8MACICELANDIC =265
   ,EL8MACGREEK =266
   ,IW8MACHEBREW =267
   ,US8ICL =277
   ,WE8ICL =278
   ,WE8ISOICLUK =279
   ,EE8EBCDIC870C =301
   ,EL8EBCDIC875S =311
   ,TR8EBCDIC1026S =312
   ,BLT8EBCDIC1112S =314
   ,IW8EBCDIC424S =315
   ,EE8EBCDIC870S =316
   ,CL8EBCDIC1025S =317
   ,TH8TISEBCDICS =319
   ,AR8EBCDIC420S =320
   ,CL8EBCDIC1025C =322
   ,WE8MACROMAN8 =351
   ,WE8MACROMAN8S =352
   ,TH8MACTHAI =353
   ,TH8MACTHAIS =354
   ,HU8CWI2 =368
   ,EL8PC437S =380
   ,EL8EBCDIC875 =381
   ,EL8PC737 =382
   ,LT8PC772 =383
   ,LT8PC774 =384
   ,EL8PC869 =385
   ,EL8PC851 =386
   ,CDN8PC863 =390
   ,HU8ABMOD =401
   ,AR8ASMO8X =500
   ,AR8NAFITHA711T =504
   ,AR8SAKHR707T =505
   ,AR8MUSSAD768T =506
   ,AR8ADOS710T =507
   ,AR8ADOS720T =508
   ,AR8APTEC715T =509
   ,AR8NAFITHA721T =511
   ,AR8HPARABIC8T =514
   ,AR8NAFITHA711 =554
   ,AR8SAKHR707 =555
   ,AR8MUSSAD768 =556
   ,AR8ADOS710 =557
   ,AR8ADOS720 =558
   ,AR8APTEC715 =559
   ,AR8MSWIN1256 =560
   ,AR8MSAWIN =560
   ,AR8NAFITHA721 =561
   ,AR8SAKHR706 =563
   ,AR8ARABICMAC =565
   ,AR8ARABICMACS =566
   ,AR8ARABICMACT =567
   ,LA8ISO6937 =590
   ,WE8DECTST =798
   ,JA16VMS =829
   ,JA16EUC =830
   ,JA16EUCYEN =831
   ,JA16SJIS =832
   ,JA16DBCS =833
   ,JA16SJISYEN =834
   ,JA16EBCDIC930 =835
   ,JA16MACSJIS =836
   ,KO16KSC5601 =840
   ,KO16DBCS =842
   ,KO16KSCCS =845
   ,KO16MSWIN949 =846
   ,ZHS16CGB231280 =850
   ,ZHS16MACCGB231280 =851
   ,ZHS16GBK =852
   ,ZHS16DBCS =853
   ,ZHT32EUC =860
   ,ZHT32SOPS =861
   ,ZHT16DBT =862
   ,ZHT32TRIS =863
   ,ZHT16DBCS =864
   ,ZHT16BIG5 =865
   ,ZHT16CCDC =866
   ,ZHT16MSWIN950 =867
   ,AL24UTFFSS =870
   ,UTF8 =871
   ,UTFE =872
   ,WE16DECTST2 =994
   ,WE16DECTST =995
   ,KO16TSTSET =996
   ,JA16TSTSET2 =997
   ,JA16TSTSET =998
   ,UTF16 =1000
   ,US16TSTFIXED =1001
   ,JA16EUCFIXED =1830
   ,JA16SJISFIXED =1832
   ,JA16DBCSFIXED =1833
   ,KO16KSC5601FIXED =1840
   ,KO16DBCSFIXED =1842
   ,ZHS16CGB231280FIXED =1850
   ,ZHS16GBKFIXED =1852
   ,ZHS16DBCSFIXED =1853
   ,ZHT32EUCFIXED =1860
   ,ZHT32TRISFIXED =1863
   ,ZHT16DBCSFIXED =1864
   ,ZHT16BIG5FIXED =1865
   ,AL16UTF16 =2000
   ,AL16UCS2  =2001
};

enum CharSetForm
{
  OCCI_SQLCS_IMPLICIT = SQLCS_IMPLICIT // use local db char set
 ,OCCI_SQLCS_NCHAR = SQLCS_NCHAR // use local db nchar set
 ,OCCI_SQLCS_EXPLICIT = SQLCS_EXPLICIT // char set explicitly specified
 ,OCCI_SQLCS_FLEXIBLE = SQLCS_FLEXIBLE // pl/sql flexible parameter
};

enum LobOpenMode
{ OCCI_LOB_READONLY = OCI_LOB_READONLY
 ,OCCI_LOB_READWRITE = OCI_LOB_READWRITE
};

class RefCounted {
public:
    RefCounted();
    virtual ~RefCounted(){} 
    const RefCounted * newRef() const;
    void deleteRef() const;

private:

    void onZeroReferences();
    unsigned long references_;
  };

template <class T> 
class ConstPtr
{

public:

ConstPtr( const T* ptr = 0 );
ConstPtr( const ConstPtr<T>& mp );
~ConstPtr();
const T * operator->() const;
const T* rawPtr() const;

#ifdef MEMBER_TEMPLATE
template<class OtherType> operator ConstPtr<OtherType>();
#endif

protected:

void operator=( const ConstPtr<T>& mp );
const T* rawPtr_;

};

template <class T>
class Ptr : public ConstPtr<T> {

public:

Ptr( T* ptr = 0 );
Ptr( const Ptr<T>& mp );
void operator=( const Ptr<T>& mp );
const T * operator->() const;
T * operator->();
T* rawPtr() ;
const T* rawPtr() const;



#ifdef MEMBER_TEMPLATE
  template<class OtherType>
  operator Ptr<OtherType>();
#endif

};


/*---------------------------------------------------------------------------
                           EXPORT FUNCTIONS
  ---------------------------------------------------------------------------*/

  void getVector( const AnyData &any, OCCI_STD_NAMESPACE::vector<OCCI_STD_NAMESPACE::string> &vect) ;
  void getVector( const AnyData &any, OCCI_STD_NAMESPACE::vector<Blob> &vect) ;
  void getVector( const AnyData &any, OCCI_STD_NAMESPACE::vector<Clob> &vect) ;
  void getVector( const AnyData &any, OCCI_STD_NAMESPACE::vector<Bfile> &vect) ;
  void getVector( const AnyData &any, OCCI_STD_NAMESPACE::vector<Number> &vect) ;
  void getVector( const AnyData &any, OCCI_STD_NAMESPACE::vector<Bytes> &vect) ;
  void getVector( const AnyData &any, OCCI_STD_NAMESPACE::vector<Date> &vect) ;
  void getVector( const AnyData &any, OCCI_STD_NAMESPACE::vector<Timestamp> &vect) ;
  template <class T>
  void getVectorOfRefs( const AnyData &any, OCCI_STD_NAMESPACE::vector< Ref<T> > &vect) ;


  #ifndef WIN32COMMON
  template <class T>
  void getVector(const AnyData &any,
  OCCI_STD_NAMESPACE::vector< Ref<T> > &vect) ;
  #endif

  #ifdef WIN32COMMON
  template <class T>
  void getVector( const AnyData &any, OCCI_STD_NAMESPACE::vector< T > &vect) ;
  #else
  template <class T>
  void getVector( const AnyData &any, OCCI_STD_NAMESPACE::vector< T* > &vect) ;
  #endif

  void setVector( AnyData &any, const OCCI_STD_NAMESPACE::vector<OCCI_STD_NAMESPACE::string> &vect) ;
  void setVector( AnyData &any, const OCCI_STD_NAMESPACE::vector<Blob> &vect) ;
  void setVector( AnyData &any, const OCCI_STD_NAMESPACE::vector<Clob> &vect) ;
  void setVector( AnyData &any, const OCCI_STD_NAMESPACE::vector<Bfile> &vect) ;
  void setVector( AnyData &any, const OCCI_STD_NAMESPACE::vector<Number> &vect) ;
  void setVector( AnyData &any, const OCCI_STD_NAMESPACE::vector<Bytes> &vect) ;
  void setVector( AnyData &any, const OCCI_STD_NAMESPACE::vector<Date> &vect) ;
  void setVector( AnyData &any, const OCCI_STD_NAMESPACE::vector<Timestamp> &vect) ;
  template <class T>
  void setVectorOfRefs( AnyData &any, const OCCI_STD_NAMESPACE::vector< Ref<T> > &vect) ;


  #ifndef WIN32COMMON
  template <class T>
  void setVector( AnyData &any, 
  const OCCI_STD_NAMESPACE::vector< Ref<T> > &vect) ;
  #endif

  #ifdef WIN32COMMON
  template <class T>
  void setVector( AnyData &any, const OCCI_STD_NAMESPACE::vector< T > &vect) ;
  #else
  template <class T>
  void setVector( AnyData &any, const OCCI_STD_NAMESPACE::vector< T* > &vect) ;
  #endif

  void getVector( ResultSet *rs, unsigned int index, 
  OCCI_STD_NAMESPACE::vector<int> &vect) ;
  void getVector( ResultSet *rs, unsigned int index, 
  OCCI_STD_NAMESPACE::vector<OCCI_STD_NAMESPACE::string> &vect) ;
  void getVector(ResultSet  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<unsigned int> &vect) ;
  void getVector(ResultSet  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<float> &vect); 
  void getVector(ResultSet  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<double> &vect);
  void getVector(ResultSet  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<Date> &vect) ;
  void getVector(ResultSet  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<Timestamp> &vect) ;
  void getVector(ResultSet  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<RefAny> &vect) ;
  void getVector(ResultSet  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<Blob> &vect) ;
  void getVector(ResultSet  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<Clob> &vect) ;
  void getVector(ResultSet  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<Bfile> &vect) ;
  void getVector(ResultSet  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<Number> &vect) ;
  void getVector(ResultSet  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<IntervalYM> &vect) ;
  void getVector(ResultSet  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<IntervalDS> &vect) ;
  template <class T>
  void getVectorOfRefs(ResultSet  *rs, unsigned int,
  OCCI_STD_NAMESPACE::vector<Ref<T> > &vect) ;

  #ifndef WIN32COMMON
  template <class T>
  void getVector(ResultSet  *rs, unsigned int,
  OCCI_STD_NAMESPACE::vector<Ref<T> > &vect) ;
  #endif

  #ifdef WIN32COMMON
  template <class T>
  void getVector( ResultSet *rs, unsigned int index,
  OCCI_STD_NAMESPACE::vector< T > &vect) ;
  #else
  template <class T>
  void getVector( ResultSet *rs, unsigned int index,
  OCCI_STD_NAMESPACE::vector< T* > &vect) ;
  #endif


  void getVector(Statement  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<RefAny> &vect) ;
  void getVector(Statement  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<Blob> &vect) ;
  void getVector(Statement  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<Clob> &vect) ;
  void getVector(Statement  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<Bfile> &vect) ;
  void getVector(Statement  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<Number> &vect) ;
  void getVector(Statement  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<IntervalYM> &vect) ;
  void getVector(Statement  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<IntervalDS> &vect) ;
  void getVector( Statement *rs, unsigned int index, 
  OCCI_STD_NAMESPACE::vector<int> &vect) ;
  void getVector( Statement *rs, unsigned int index,
  OCCI_STD_NAMESPACE::vector<OCCI_STD_NAMESPACE::string> &vect) ;
  void getVector(Statement  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<unsigned int> &vect) ;
  void getVector(Statement  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<float> &vect) ;
  void getVector(Statement  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<double> &vect) ;
  void getVector(Statement  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<Date> &vect) ;
  void getVector(Statement  *rs, unsigned int, 
  OCCI_STD_NAMESPACE::vector<Timestamp> &vect) ;
  template <class T>
  void getVectorOfRefs(Statement  *rs, unsigned int,
  OCCI_STD_NAMESPACE::vector<Ref<T> > &vect) ;

  #ifndef WIN32COMMON
  template <class T>
  void getVector(Statement  *rs, unsigned int,
  OCCI_STD_NAMESPACE::vector<Ref<T> > &vect) ;
  #endif

  #ifdef WIN32COMMON
  template <class T>
  void getVector( Statement *rs, unsigned int index,
  OCCI_STD_NAMESPACE::vector< T > &vect) ;
  #else
  template <class T>
  void getVector( Statement *rs, unsigned int index,
  OCCI_STD_NAMESPACE::vector< T* > &vect) ;
  #endif


  void setVector(Statement *stmt, unsigned int paramIndex, 
  const OCCI_STD_NAMESPACE::vector<int> &vect, const OCCI_STD_NAMESPACE::string &sqltype) ;
  void setVector(Statement *stmt, unsigned int paramIndex, 
  const OCCI_STD_NAMESPACE::vector<unsigned int> &vect, const OCCI_STD_NAMESPACE::string &sqltype) ;
  void setVector(Statement *stmt, unsigned int paramIndex, 
  const OCCI_STD_NAMESPACE::vector<double> &vect, const OCCI_STD_NAMESPACE::string &sqltype) ;
  void setVector(Statement *stmt, unsigned int paramIndex, 
  const OCCI_STD_NAMESPACE::vector<float> &vect, const OCCI_STD_NAMESPACE::string &sqltype) ;
 void setVector(Statement *stmt, unsigned int paramIndex,
 const OCCI_STD_NAMESPACE::vector<Number> &vect,
 const OCCI_STD_NAMESPACE::string &sqltype);
  void setVector(Statement *stmt, unsigned int paramIndex, 
  const OCCI_STD_NAMESPACE::vector<OCCI_STD_NAMESPACE::string> &vect, const OCCI_STD_NAMESPACE::string &sqltype) ;
  void setVector(Statement *stmt, unsigned int paramIndex, 
  const OCCI_STD_NAMESPACE::vector<RefAny> &vect, const OCCI_STD_NAMESPACE::string &sqltype) ;
  void setVector(Statement *stmt, unsigned int paramIndex, 
  const OCCI_STD_NAMESPACE::vector<Blob> &vect, const OCCI_STD_NAMESPACE::string &sqltype) ;
  void setVector(Statement *stmt, unsigned int paramIndex, 
  const OCCI_STD_NAMESPACE::vector<Clob> &vect, const OCCI_STD_NAMESPACE::string &sqltype) ;
  void setVector(Statement *stmt, unsigned int paramIndex, 
  const OCCI_STD_NAMESPACE::vector<Bfile> &vect, const OCCI_STD_NAMESPACE::string &sqltype) ;
  void setVector(Statement *stmt, unsigned int paramIndex, 
  const OCCI_STD_NAMESPACE::vector<Timestamp> &vect, const OCCI_STD_NAMESPACE::string &sqltype) ;
  void setVector(Statement *stmt, unsigned int paramIndex, 
  const OCCI_STD_NAMESPACE::vector<IntervalDS> &vect, const OCCI_STD_NAMESPACE::string &sqltype) ;
  void setVector(Statement *stmt, unsigned int paramIndex, 
  const OCCI_STD_NAMESPACE::vector<IntervalYM> &vect, const OCCI_STD_NAMESPACE::string &sqltype) ;
  void setVector(Statement *stmt, unsigned int paramIndex, 
  const OCCI_STD_NAMESPACE::vector<Date> &vect, const OCCI_STD_NAMESPACE::string &sqltype) ;
  template  <class T>
  void setVectorOfRefs(Statement *stmt, unsigned int paramIndex,
  const OCCI_STD_NAMESPACE::vector<Ref<T> > &vect,
  const OCCI_STD_NAMESPACE::string &sqltype) ;

  #ifndef WIN32COMMON
  template  <class T>
  void setVector(Statement *stmt, unsigned int paramIndex,
  const OCCI_STD_NAMESPACE::vector<Ref<T> > &vect,
  const OCCI_STD_NAMESPACE::string &sqltype) ;
  #endif

  #ifdef WIN32COMMON
  template <class T>
  void setVector( Statement *stmt, unsigned int paramIndex, 
  const OCCI_STD_NAMESPACE::vector< T > &vect, const OCCI_STD_NAMESPACE::string   &sqltype) ;
  #else
  template <class T>
  void setVector( Statement *stmt, unsigned int paramIndex,
  const OCCI_STD_NAMESPACE::vector<T* > &vect, const OCCI_STD_NAMESPACE::string
  &sqltype) ;
  #endif


/*---------------------------------------------------------------------------
                          INTERNAL FUNCTIONS
  ---------------------------------------------------------------------------*/


} /* end of namespace occi */
} /* end of namespace oracle */
#endif /* _olint */

#endif                                              /* OCCICOMMON_ORACLE */
