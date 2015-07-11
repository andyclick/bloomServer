/*--   
    
  Copyright   (c)   2004   Shenzhen   Huaren   Education   Co.Ltd   
    
  File   Name:   
  config.h   
  Version:   
  2.0   
  Abstract:   
  declarations/definitions   for   reading   configuration   file.   
  Author:   
  Gang   He   
  Created   on:   
  2003-12-23   
  Modified   History:   
  2004-09-02   
  Modified   Person:   
    
  --*/   
    
  #ifndef   _HEGANG_CONFIG_H_   
  #define   _HEGANG_CONFIG_H_   
    
  #ifdef   __cplusplus   
  extern   "C"   {   
  #endif   
    
    
  /*   Define   return   error   code   value   */   
  #define   CFG_NOERROR   0 /*   read   configuration   file   successfully   */   
  #define   CFG_NOFILE -1 /*   not   find   or   open   configuration   file   */   
  #define   CFG_NOFIND -2 /*   not   find   section   or   key   name   in   configuration   file   */   
    
    
  /*-   
  Name:   
  int   getconfigstr(const   char*     section,   
      const   char*     keyname,   
      char*               keyvalue,   
      unsigned   int   len,   
      const   char*     filename);   
  Description:   
  Read   the   value   of   key   name   in   string   format.     
  Input   Parameters:   
  section:   section   name   
  keyname:   key   name   
  len:   size   of   destination   buffer   
  filename:   configuration   filename   
  Output   Parameters:   
  keyvalue:   destination   buffer   
  Return   Value:   
  0,   otherwise   errno   is   returned   if   an   error   occurred.   
  -*/   
  int   getconfigstr(const   char*     section,   
    const   char*     keyname,   
    char*               keyvalue,   
    unsigned   int   len,   
    const   char*     filename);   
    
    
  /*-   
  Name:   
  int   getconfigint(const   char*     section,   
      const   char*     keyname,   
      int*                   keyvalue,   
      const   char*     filename);     
  Description:   
  Read   the   value   of   key   name   in   integer   format.     
  Input   Parameters:   
  section:   section   name   
  keyname:   key   name   
  filename:   configuration   filename   
  Output   Parameters:   
  keyvalue:   destination   buffer   
  Return   Value:   
  0,   otherwise   errno   is   returned   if   an   error   occurred.   
  -*/   
  int   getconfigint(const   char*     section,   
    const   char*     keyname,   
    int*                   keyvalue,   
    const   char*     filename);     
    
    
  #ifdef   __cplusplus   
  }   
  #endif   
    
  #endif   
