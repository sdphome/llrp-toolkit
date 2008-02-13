<?xml version="1.0" encoding="UTF-8" ?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
	xmlns:llrp="http://www.llrp.org/ltk/schema/core/encoding/binary/1.0"
  xmlns:h="http://www.w3.org/1999/xhtml">
  <xsl:output omit-xml-declaration='yes' method='text' indent='yes'/>
  
  <xsl:template match="/llrp:llrpdef">
    /*
    ***************************************************************************
    *  Copyright 2007 Impinj, Inc.
    *
    *  Licensed under the Apache License, Version 2.0 (the "License");
    *  you may not use this file except in compliance with the License.
    *  You may obtain a copy of the License at
    *
    *      http://www.apache.org/licenses/LICENSE-2.0
    *
    *  Unless required by applicable law or agreed to in writing, software
    *  distributed under the License is distributed on an "AS IS" BASIS,
    *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    *  See the License for the specific language governing permissions and
    *  limitations under the License.
    *
    ***************************************************************************
    */


    /*
    ***************************************************************************
    *
    *  This code is generated by Impinj LLRP .Net generator. Modification is
    *  not recommended.
    *
    ***************************************************************************
    */

    /*
    ***************************************************************************
    * File Name:       LLRPMsg.cs
    *
    * Version:         1.0
    * Author:          Impinj
    * Organization:    Impinj
    * Date:            Jan. 18, 2008
    *
    * Description:     This file contains LLRP message definitions
    ***************************************************************************
    */

    using System;
    using System.IO;
    using System.Text;
    using System.Collections;
    using System.ComponentModel;
    using System.Xml;
    using System.Xml.Serialization;
    using System.Xml.Schema;
    using System.Runtime.InteropServices;

    using LLRP.DataType;

    namespace LLRP
    {

    #region Custom Parameter Interface
    <xsl:for-each select ="llrp:messageDefinition">
      <xsl:variable name="custom_msg_name">
        <xsl:value-of select="@name"/>
      </xsl:variable>
      <xsl:for-each select="llrp:parameter">
        <xsl:if test="@type='Custom'">
          public interface I<xsl:copy-of select="$custom_msg_name"/>_Custom_Param : ICustom_Parameter {}
        </xsl:if>
      </xsl:for-each>
    </xsl:for-each>
    #endregion
    
    //LLRP message definitions
    <xsl:for-each select="llrp:messageDefinition">
        /// <xsl:text disable-output-escaping="yes">&lt;summary</xsl:text><xsl:text disable-output-escaping="yes">&gt;</xsl:text>
        /// <xsl:for-each select ="llrp:annotation/llrp:description/h:p"><xsl:value-of select="."/></xsl:for-each> 
        /// <xsl:text disable-output-escaping="yes">&lt;/summary</xsl:text><xsl:text disable-output-escaping="yes">&gt;</xsl:text>
      <xsl:variable name="msg_name">
        <xsl:value-of select="@name"/>
      </xsl:variable>
      public class MSG_<xsl:value-of select="@name"/> : Message
      {
        protected new static UInt32 sequence_num = 0;                     //used for generating message id automatically

      <xsl:for-each select="*">
        <xsl:if test="name()='field'">
          public <xsl:call-template name='DefineDataType'/><xsl:text> </xsl:text><xsl:value-of select="@name"/><xsl:call-template name='DefineDefaultValue'/>
          <xsl:call-template name="DefineDataLength"/>
        </xsl:if>
        <xsl:if test="name()='reserved'">
          private const UInt16 param_reserved_len = <xsl:value-of select="@bitCount"/>;
        </xsl:if>
        <xsl:if test="name()='parameter'">
          <xsl:choose>
            <xsl:when test="@type='Custom'">
              <xsl:choose>
                <xsl:when test="contains(@repeat, '0-N') or contains(@repeat, '1-N')">
                  public readonly CustomParameterArrayList <xsl:call-template name='DefineParameterName'/> = new CustomParameterArrayList();
                  public void AddCustomParameter(I<xsl:copy-of select='$msg_name'/>_Custom_Param param)
                  {
                  <xsl:call-template name='DefineParameterName'/>.Add(param);
                  }
                  private void AddCustomParameter(ICustom_Parameter param)
                  {
                  <xsl:call-template name='DefineParameterName'/>.Add(param);
                  }
                </xsl:when>
                <xsl:otherwise>
                  public readonly I<xsl:copy-of select='$msg_name'/>_Custom_Param <xsl:call-template name='DefineParameterName'/>;
                </xsl:otherwise>
              </xsl:choose>
            </xsl:when>
            <xsl:otherwise>
              <xsl:choose>
                <xsl:when test="contains(@repeat, '0-N') or contains(@repeat, '1-N')">
                  public PARAM_<xsl:value-of select="@type"/>[] <xsl:call-template name='DefineParameterName'/>;
                </xsl:when>
                <xsl:otherwise>
                  public PARAM_<xsl:value-of select="@type"/><xsl:text> </xsl:text><xsl:call-template name='DefineParameterName'/>;
              </xsl:otherwise>
            </xsl:choose>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:if>
      <xsl:if test="name()='choice'">
        public UNION_<xsl:value-of select="@type"/><xsl:text> </xsl:text><xsl:call-template name='DefineParameterName'/> = new UNION_<xsl:value-of select="@type"/>();
          <!--public ENUM_<xsl:value-of select="@type"/>_TYPE <xsl:call-template name='DefineParameterName'/>_type;-->
        </xsl:if>
      </xsl:for-each>

        public MSG_<xsl:value-of select="@name"/>(){msgType = <xsl:value-of select="@typeNum"/>; sequence_num++;}

      <xsl:call-template name="EncodeToBitArray"/>
      <xsl:call-template name="DecodeFromBitArray"/>
      <xsl:call-template name="ToString"/>
      <xsl:call-template name="FromString"/>
      
     }
    </xsl:for-each>
    }
  </xsl:template>

  <xsl:include href="templates.xslt"/>

  <xsl:template name="DecodeFromBitArray">
    public new static MSG_<xsl:value-of select="@name"/> FromBitArray(ref BitArray bit_array, ref int cursor, int length)
    {
    if(cursor<xsl:text disable-output-escaping="yes">&gt;</xsl:text>length)return null;

    UInt16 loop_control_counter = 1;    //used for control choice element parsing loop
    
    int field_len = 0;
    object obj_val;
    ArrayList param_list = new ArrayList();

    MSG_<xsl:value-of select="@name"/> obj = new MSG_<xsl:value-of select="@name"/>();

    int msg_type = 0;
    cursor += 6;
    msg_type = (int)(UInt64)Util.CalculateVal(ref bit_array, ref cursor, 10);

    if(msg_type!=obj.msgType)
    {
    cursor -=16;
    return null;
    }

    obj.msgLen = (UInt32)(UInt64)Util.CalculateVal(ref bit_array, ref cursor, 32);
    obj.msgID = (UInt32)(UInt64)Util.CalculateVal(ref bit_array, ref cursor, 32);

    <xsl:for-each select="*">
      <xsl:if test="name()='field'">
        if(cursor<xsl:text disable-output-escaping="yes">&gt;</xsl:text>length)throw new Exception("Input data is not a complete LLRP message");
        <xsl:if test="@type='u1v' or @type='u8v' or @type='u16v' or @type='u32v' or @type='utf8v'">
          field_len = Util.DetermineFieldLength(ref bit_array, ref cursor);
        </xsl:if>
        <xsl:if test="@type='bytesToEnd'">
          field_len = (bit_array.Length - cursor)/8;
        </xsl:if>
        <xsl:if test="@type='u96'">
          field_len = 96;
        </xsl:if>
        <xsl:if test="@type='u2'">
          field_len = 2;
        </xsl:if>
        <xsl:if test="@type='u1'">
          field_len = 1;
        </xsl:if>
        <xsl:if test="@type='u8' or @type='s8'">
          field_len = 8;
        </xsl:if>
        <xsl:if test="@type='u16' or @type='s16'">
          field_len = 16;
        </xsl:if>
        <xsl:if test="@type='u32'">
          field_len = 32;
        </xsl:if>
        <xsl:if test="@type='u64'">
          field_len = 64;
        </xsl:if>
        <xsl:choose>
          <xsl:when test="@enumaration">
            UInt32 val;
            Util.ConvertBitArrayToObj(ref bit_array, ref cursor, out val, typeof(UInt32), field_len);
            obj.<xsl:value-of select="@name"/> = (<xsl:call-template name='DefineDataType'/>)val;
          </xsl:when>
          <xsl:otherwise>
            Util.ConvertBitArrayToObj(ref bit_array, ref cursor, out obj_val, typeof(<xsl:call-template name='DefineDataType'/>), field_len);
            obj.<xsl:value-of select="@name"/> = (<xsl:call-template name='DefineDataType'/>)obj_val;
          </xsl:otherwise>
        </xsl:choose>
      </xsl:if>
      <xsl:if test="name()='reserved'">
        cursor += param_reserved_len;
      </xsl:if>
      <xsl:if test="name()='parameter'">
        <xsl:choose>
          <xsl:when test="@type='Custom'">
            <xsl:if test="@repeat = '1-N' or @repeat = '0-N'">
              ICustom_Parameter custom = CustomParamDecodeFactory.DecodeCustomParameter(ref bit_array, ref cursor, length);
              if(custom!=null)
              {
              obj.<xsl:call-template name='DefineParameterName'/>.Add(custom);
              while((custom = CustomParamDecodeFactory.DecodeCustomParameter(ref bit_array, ref cursor, length))!=null)obj.<xsl:call-template name='DefineParameterName'/>.Add(custom);
              }
            </xsl:if>
            <xsl:if test="@repeat = '1' or @repeat='0-1'">
              obj.<xsl:call-template name='DefineParameterName'/> = CustomParamDecodeFactory.DecodeCustomParameter(ref bit_array, ref cursor, length);
            </xsl:if>
          </xsl:when>
          <xsl:otherwise>
            <xsl:if test="@repeat = '1-N' or @repeat = '0-N'">
              param_list = new ArrayList();
              PARAM_<xsl:value-of select="@type"/> _param_<xsl:value-of select="@type"/> =  PARAM_<xsl:value-of select="@type"/>.FromBitArray(ref bit_array, ref cursor, length);
              if(_param_<xsl:value-of select="@type"/>!=null)
              {param_list.Add(_param_<xsl:value-of select="@type"/>);
              while((_param_<xsl:value-of select="@type"/>=PARAM_<xsl:value-of select="@type"/>.FromBitArray(ref bit_array, ref cursor, length))!=null)param_list.Add(_param_<xsl:value-of select="@type"/>);
              if(param_list.Count<xsl:text disable-output-escaping="yes">&gt;</xsl:text>0)
              {
              obj.<xsl:call-template name='DefineParameterName'/> = new PARAM_<xsl:value-of select="@type"/>[param_list.Count];
              for(int i=0;i<xsl:text disable-output-escaping="yes">&lt;</xsl:text>param_list.Count;i++)
              obj.<xsl:call-template name='DefineParameterName'/>[i] = (PARAM_<xsl:value-of select="@type"/>)param_list[i];
              }
              }
            </xsl:if>
            <xsl:if test="@repeat = '1' or @repeat='0-1'">
              obj.<xsl:call-template name='DefineParameterName'/> = PARAM_<xsl:value-of select="@type"/>.FromBitArray(ref bit_array, ref cursor, length);
            </xsl:if>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:if>
      <xsl:if test="name()='choice'">
        <xsl:variable name="choiceParameterName">
          <xsl:call-template name='DefineParameterName'/>
        </xsl:variable>
        loop_control_counter = 1;
        while(loop_control_counter!=1)
        {
        loop_control_counter = 0;
        <xsl:for-each select='../../llrp:choiceDefinition'>
          <xsl:if test='@name=$choiceParameterName'>
            <xsl:for-each select='*'>
              PARAM_<xsl:value-of select='@type'/> _param_<xsl:value-of select='@type'/> = PARAM_<xsl:value-of select='@type'/>.FromBitArray(ref bit_array, ref cursor, length);
              if(_param_<xsl:value-of select='@type'/>!=null)
              {
              loop_control_counter++;
              obj.<xsl:copy-of select='$choiceParameterName'/>.Add(_param_<xsl:value-of select='@type'/>);
              }
              while((_param_<xsl:value-of select='@type'/> = PARAM_<xsl:value-of select='@type'/>.FromBitArray(ref bit_array, ref cursor, length))!=null){
              obj.<xsl:copy-of select='$choiceParameterName'/>.Add(_param_<xsl:value-of select='@type'/>);
              }
            </xsl:for-each>
          </xsl:if>
        </xsl:for-each>
        }
      </xsl:if>
    </xsl:for-each>
    return obj;
    }
  </xsl:template>

  <xsl:template name="EncodeToBitArray">
    public override bool[] ToBitArray()
    {
    int len = 0;
    int cursor = 0;
    bool[] bit_array = new bool[500*1024*8];

    msgID=sequence_num;

    BitArray bArr = Util.ConvertIntToBitArray(version, 3);
    cursor+=3;
    bArr.CopyTo(bit_array, cursor);

    cursor+=3;
    bArr = Util.ConvertIntToBitArray(msgType, 10);
    bArr.CopyTo(bit_array, cursor);

    cursor+=10;
    bArr = Util.ConvertIntToBitArray(msgLen ,32);
    bArr.CopyTo(bit_array, cursor);

    cursor+=32;
    bArr = Util.ConvertIntToBitArray(msgID,32);
    bArr.CopyTo(bit_array, cursor);

    cursor+=32;
    <xsl:for-each select="*">
      <xsl:if test="name()='field'">
        if(<xsl:value-of select="@name"/>!=null)
        {
        try
        {
        BitArray tempBitArr = Util.ConvertObjToBitArray(<xsl:value-of select="@name"/>, <xsl:value-of select="@name"/>_len);
        tempBitArr.CopyTo(bit_array, cursor);
        cursor += tempBitArr.Length;
        }
        catch{cursor +=<xsl:value-of select="@name"/>_len;}
        }
      </xsl:if>
      <xsl:if test="name()='reserved'">
        cursor += param_reserved_len;
      </xsl:if>
      <xsl:if test="name()='parameter'">
        if(<xsl:call-template name='DefineParameterName'/> != null)
        {
        <xsl:choose>
          <xsl:when test="@repeat = '0-N' or @repeat = '1-N'">
            len = <xsl:call-template name='DefineParameterName'/>.Length;
            for(int i=0; i<xsl:text disable-output-escaping="yes">&lt;</xsl:text>len;i++)
            <xsl:call-template name='DefineParameterName'/>[i].ToBitArray(ref bit_array, ref cursor);
          </xsl:when>
          <xsl:otherwise>
            <xsl:call-template name='DefineParameterName'/>.ToBitArray(ref bit_array, ref cursor);
          </xsl:otherwise>
        </xsl:choose>
        }
      </xsl:if>
      <xsl:if test="name()='choice'">
        <xsl:variable name="choiceParameterName">
          <xsl:call-template name='DefineParameterName'/>
        </xsl:variable>
        len = <xsl:copy-of select='$choiceParameterName'/>.Count;
        for(int i=0; i<xsl:text disable-output-escaping="yes">&lt;</xsl:text>len;i++)<xsl:copy-of select='$choiceParameterName'/>[i].ToBitArray(ref bit_array, ref cursor);
      </xsl:if>
    </xsl:for-each>

    UInt32 msg_len = (UInt32)cursor/8;
    bArr = Util.ConvertIntToBitArray(msg_len ,32);
    bArr.CopyTo(bit_array, 16);

    bool[] boolArr = new bool[cursor];
    Array.Copy(bit_array, 0, boolArr, 0, cursor);
    return boolArr;
    }
  </xsl:template>

  <xsl:template name ="ToString">
    public override string ToString()
    {
    int len;
    string xml_str = "<xsl:text disable-output-escaping="yes">&lt;</xsl:text><xsl:value-of select="@name"/>"+ " Version=\"" + version.ToString() + "\" MessageID=\"" + MSG_ID.ToString() + "\"" + "<xsl:text disable-output-escaping="yes">&gt;</xsl:text>";
    <xsl:for-each select="*">
      <xsl:choose>
        <xsl:when test="name()='field'">
          if(<xsl:value-of select="@name"/>!=null)
          {
          <xsl:choose>
            <xsl:when test ="@format='Hex'">
              xml_str +="<xsl:text disable-output-escaping="yes">&lt;</xsl:text><xsl:value-of select="@name"/><xsl:text disable-output-escaping="yes">&gt;</xsl:text>" + <xsl:value-of select="@name"/>.ToHexString() + "<xsl:text disable-output-escaping="yes">&lt;/</xsl:text><xsl:value-of select="@name"/><xsl:text disable-output-escaping="yes">&gt;</xsl:text>";
            </xsl:when>
            <xsl:otherwise>
              xml_str +="<xsl:text disable-output-escaping="yes">&lt;</xsl:text><xsl:value-of select="@name"/><xsl:text disable-output-escaping="yes">&gt;</xsl:text>" + <xsl:value-of select="@name"/>.ToString() + "<xsl:text disable-output-escaping="yes">&lt;/</xsl:text><xsl:value-of select="@name"/><xsl:text disable-output-escaping="yes">&gt;</xsl:text>";
            </xsl:otherwise>
          </xsl:choose>
          }
        </xsl:when>
        <xsl:when test="name()='parameter'">
          if(<xsl:call-template name='DefineParameterName'/>!= null)
          {
          <xsl:choose>
            <xsl:when test="@type='Custom'">
              <!--xml_str += "<xsl:text disable-output-escaping="yes">&lt;</xsl:text>Custom<xsl:text disable-output-escaping="yes">&gt;</xsl:text>";-->
              <xsl:choose>
                <xsl:when test="@repeat = '0-N' or @repeat = '1-N'">
                  len = <xsl:call-template name='DefineParameterName'/>.Length;
                  for(int i=0; i<xsl:text disable-output-escaping="yes">&lt;</xsl:text>len;i++)
                  xml_str += <xsl:call-template name='DefineParameterName'/>[i].ToString();
                </xsl:when>
                <xsl:otherwise>
                  xml_str += <xsl:call-template name='DefineParameterName'/>.ToString();
                </xsl:otherwise>
              </xsl:choose>
              <!--xml_str += "<xsl:text disable-output-escaping="yes">&lt;</xsl:text>/Custom<xsl:text disable-output-escaping="yes">&gt;</xsl:text>";-->
            </xsl:when>
            <xsl:otherwise>
              <xsl:choose>
                <xsl:when test="@repeat = '0-N' or @repeat = '1-N'">
                  len = <xsl:call-template name='DefineParameterName'/>.Length;
                  for(int i=0; i<xsl:text disable-output-escaping="yes">&lt;</xsl:text>len;i++)
                  xml_str += <xsl:call-template name='DefineParameterName'/>[i].ToString();
                </xsl:when>
                <xsl:otherwise>
                  xml_str += <xsl:call-template name='DefineParameterName'/>.ToString();
                </xsl:otherwise>
              </xsl:choose>
            </xsl:otherwise>
          </xsl:choose>
          }
        </xsl:when>
        <xsl:when test="name()='choice'">
          <xsl:variable name="choiceParameterName">
            <xsl:call-template name='DefineParameterName'/>
          </xsl:variable>
          if(<xsl:call-template name='DefineParameterName'/>!= null)
          {
          len = <xsl:copy-of select='$choiceParameterName'/>.Count;
          for(int i=0; i<xsl:text disable-output-escaping="yes">&lt;</xsl:text>len;i++)xml_str += <xsl:copy-of select='$choiceParameterName'/>[i].ToString();
          }
        </xsl:when>
        <xsl:otherwise>

        </xsl:otherwise>
      </xsl:choose>
    </xsl:for-each>
    xml_str += "<xsl:text disable-output-escaping="yes">&lt;/</xsl:text><xsl:value-of select="@name"/><xsl:text disable-output-escaping="yes">&gt;</xsl:text>";
    return xml_str;
    }
  </xsl:template>

  <xsl:template name ="FromString">
    public new static MSG_<xsl:value-of select="@name"/>  FromString(string str)
    {
    string val;

    XmlDocument xdoc = new XmlDocument();
    xdoc.LoadXml(str);
    XmlNode node = (XmlNode)xdoc.DocumentElement;

    MSG_<xsl:value-of select="@name"/> msg = new MSG_<xsl:value-of select="@name"/>();
    try{msg.MSG_ID = Convert.ToUInt32(XmlUtil.GetNodeAttrValue(node, "MessageID"));}catch{}

    <xsl:for-each select="*">
      <xsl:choose>
        <xsl:when test="name()='field'">
          val = XmlUtil.GetNodeValue(node, "<xsl:value-of select="@name"/>");
          <xsl:choose>
            <xsl:when test="@enumeration">
              msg.<xsl:value-of select="@name"/> = (<xsl:call-template name='DefineDataType'/>)Enum.Parse(typeof(<xsl:call-template name='DefineDataType'/>), val);
            </xsl:when>
            <xsl:otherwise>
              <xsl:if test="@type='u1v' or @type='u2' or @type='u8v' or @type='u16v' or @type='u32v' or @type='u96' or @type='bytesToEnd'">
                msg.<xsl:value-of select="@name"/> = <xsl:call-template name='DefineDataType'/>.FromString(val);
              </xsl:if>
              <xsl:if test="@type='u1'">
                msg.<xsl:value-of select="@name"/> = Convert.ToBoolean(val);
              </xsl:if>
              <xsl:if test="@type='u8'">
                msg.<xsl:value-of select="@name"/> = Convert.ToByte(val);
              </xsl:if>
              <xsl:if test="@type='s8'">
                msg.<xsl:value-of select="@name"/> = Convert.ToSByte(val);
              </xsl:if>
              <xsl:if test="@type='u16'">
                msg.<xsl:value-of select="@name"/> = Convert.ToUInt16(val);
              </xsl:if>
              <xsl:if test="@type='s16'">
                msg.<xsl:value-of select="@name"/> = Convert.ToInt16(val);
              </xsl:if>
              <xsl:if test="@type='u32'">
                msg.<xsl:value-of select="@name"/> = Convert.ToUInt32(val);
              </xsl:if>
              <xsl:if test="@type='u64'">
                msg.<xsl:value-of select="@name"/> = Convert.ToUInt64(val);
              </xsl:if>
            </xsl:otherwise>
          </xsl:choose>
        </xsl:when>
        <xsl:when test="name()='parameter'">
          try
          {
          <xsl:choose>
            <xsl:when test="@type='Custom'">
              <xsl:choose>
                <xsl:when test="@repeat = '0-N' or @repeat = '1-N'">
                  ArrayList xnl = XmlUtil.GetXmlNodeCustomChildren(node);
                  if(null!=xnl)
                  {
                  if(xnl.Count!=0)
                  {
                  for(int i=0; i<xsl:text disable-output-escaping="yes">&lt;</xsl:text>xnl.Count; i++)
                  {
                  ICustom_Parameter custom = CustomParamDecodeFactory.DecodeXmlNodeToCustomParameter((XmlNode)xnl[i]);
                  if(custom!=null)msg.AddCustomParameter(custom);
                  }
                  }
                  }
                </xsl:when>
                <xsl:otherwise>
                  ArrayList xnl = XmlUtil.GetXmlNodeCustomChildren(node);
                  if(null!=xnl)
                  {
                  if(xnl.Count!=0)
                  msg.<xsl:call-template name='DefineParameterName'/> = CustomParamDecodeFactory.DecodeXmlNodeToCustomParameter((XmlNode)xnl[0]);
                  }
                </xsl:otherwise>
              </xsl:choose>
            </xsl:when>
            <xsl:otherwise>
              <xsl:choose>
                <xsl:when test="@repeat = '0-N' or @repeat = '1-N'">
                  XmlNodeList xnl = XmlUtil.GetXmlNodes(node, "<xsl:call-template name='DefineParameterName'/>");
                  if(null!=xnl)
                  {
                  if(xnl.Count!=0)
                  {
                  msg.<xsl:call-template name='DefineParameterName'/> = new PARAM_<xsl:value-of select="@type"/>[xnl.Count];
                  for(int i=0; i<xsl:text disable-output-escaping="yes">&lt;</xsl:text>xnl.Count; i++)
                  msg.<xsl:call-template name='DefineParameterName'/>[i] = PARAM_<xsl:value-of select="@type"/>.FromXmlNode(xnl[i]);
                  }
                  }
                </xsl:when>
                <xsl:otherwise>
                  XmlNodeList xnl = XmlUtil.GetXmlNodes(node, "<xsl:call-template name='DefineParameterName'/>");
                  if(null!=xnl)
                  {
                  if(xnl.Count!=0)
                  msg.<xsl:call-template name='DefineParameterName'/> = PARAM_<xsl:value-of select="@type"/>.FromXmlNode(xnl[0]);
                  }
                </xsl:otherwise>
              </xsl:choose>
            </xsl:otherwise>
          </xsl:choose>
          }catch{}
        </xsl:when>
        <xsl:when test="name()='choice'">
          {
          <xsl:variable name="choiceParameterName">
            <xsl:call-template name='DefineParameterName'/>
          </xsl:variable>
          msg.<xsl:copy-of select='$choiceParameterName'/> = new UNION_<xsl:value-of select="@type"/>();
          <xsl:for-each select='../../llrp:choiceDefinition'>
            <xsl:if test='@name=$choiceParameterName'>
              <xsl:for-each select='*'>
                XmlNodeList xnl = XmlUtil.GetXmlNodes(node, "<xsl:call-template name='DefineParameterName'/>");
                if(xnl.Count!=0)
                {
                <!--msg.<xsl:copy-of select='$choiceParameterName'/>_type = ENUM_<xsl:copy-of select='$choiceParameterName'/>_TYPE.<xsl:value-of select='@type'/>;-->
                for(int i=0; i<xsl:text disable-output-escaping="yes">&lt;</xsl:text>xnl.Count; i++)
                msg.<xsl:copy-of select='$choiceParameterName'/>.Add(PARAM_<xsl:value-of select='@type'/>.FromXmlNode(xnl[i]));
                }
              </xsl:for-each>
            </xsl:if>
          </xsl:for-each>
          }
        </xsl:when>
        <xsl:otherwise>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:for-each>
    return msg;
    }
  </xsl:template>
  
</xsl:stylesheet>