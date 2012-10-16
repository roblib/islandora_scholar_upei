<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.loc.gov/mods/v3"
    version="1.0">
    <xsl:strip-space elements="*"/>
    <xsl:output method="xml" indent="yes"/>
    <xsl:template match="/">
        <mods>
            <titleInfo>                
                <!--               <title>
                    <xsl:variable name="title-clean">
                        <xsl:call-template name="string-replace-all">
                            <xsl:with-param name="text" select="//reference/t1"/>
                            <xsl:with-param name="replace" select="'&amp;#39;'"/>
                            <xsl:with-param name="by" select="'&amp;apos;'"/>
                            
                        </xsl:call-template>
                    </xsl:variable>
                    <xsl:value-of select="$title-clean"/>
                </title>
 
                <subTitle>
                    <xsl:value-of select="//reference/t2"/>
                </subTitle>
 -->
                <xsl:for-each select="//reference/t1">
                    <xsl:call-template name="title">
                        <xsl:with-param name="strTitle" select="."/>
                    </xsl:call-template>
                </xsl:for-each>            
            </titleInfo>
             
            <xsl:for-each select="//reference/a1">
                <name>
                    <xsl:choose>
                        <xsl:when test="contains(text(), ',')">
                            <xsl:attribute name="type">personal</xsl:attribute>
                            <namePart>
                                <xsl:attribute name="type">given</xsl:attribute>
                                <xsl:value-of select="normalize-space(substring-after(text(),','))"/>
                            </namePart>
                            <namePart>
                                <xsl:attribute name="type">family</xsl:attribute>
                                <xsl:value-of select="normalize-space(substring-before(text(),','))"/>
                            </namePart>

                        </xsl:when>
                        <xsl:otherwise>
                            <xsl:attribute name="type">corporate</xsl:attribute>
                            <namePart>
                                <!--<xsl:attribute name="type">family</xsl:attribute>--> 
                                <xsl:value-of select="normalize-space(text())"/>
                            </namePart>
                        </xsl:otherwise>
                    </xsl:choose>
                    <role>
                        <roleTerm>
                            <xsl:attribute name="authority">marcrelator</xsl:attribute>
                            <xsl:attribute name="type">text</xsl:attribute>
                            <xsl:text>Author</xsl:text>
                        </roleTerm>
                    </role>
                </name>
            </xsl:for-each>
            <xsl:for-each select="//reference/u1">
                <xsl:call-template name="links">
                    <xsl:with-param name="str" select="."/>
                </xsl:call-template>
            </xsl:for-each>
            <xsl:for-each select="//reference/u2">
                <xsl:call-template name="links2">
                    <xsl:with-param name="str" select="."/>
                </xsl:call-template>
            </xsl:for-each>
            <typeOfResource>text</typeOfResource>
            <genre>
                <xsl:value-of select="//reference/rt"/>
            </genre>
            <identifier>
                <xsl:attribute name="type">refworks</xsl:attribute>
                <xsl:value-of select="//reference/id"/>
            </identifier>
            <subject authority="local">
                <xsl:for-each select="//reference/k1">
                    <topic>
                        <xsl:value-of select="normalize-space(text())"/>
                    </topic>
                </xsl:for-each>
            </subject>
            <abstract>
                <xsl:variable name="abstract-clean">
                    <xsl:call-template name="string-replace-all">
                        <xsl:with-param name="text" select="//reference/ab"/>
                        <xsl:with-param name="replace" select="'&amp;#39;'"/>
                        <xsl:with-param name="by" select="'&amp;apos;'"/>
                    </xsl:call-template>
                </xsl:variable>
                <xsl:value-of select="$abstract-clean"/>
            </abstract>
            <xsl:if test="//reference/no/text() [normalize-space(.) ]">
                <note>
                    <xsl:value-of select="//reference/no"/>
                </note>
            </xsl:if>
            <note>Source type: <xsl:value-of select="//reference/sr"/></note>
            <xsl:if test="//reference/lk/text() [normalize-space(.) ]">
                <note>
                    <xsl:value-of select="//reference/lk"/>
                </note>
            </xsl:if>
            <location>
                <url>
                    <xsl:value-of select="//reference/ul"/>
                </url>
            </location>
            <xsl:if test="//reference/ol/text() [normalize-space(.) ]">
            <xsl:choose>
                <xsl:when test="//reference/ol/text()='Unknown(0)'"/>
                    <xsl:otherwise>
                    <language>
                        <languageTerm>
                            <xsl:attribute name="type">code</xsl:attribute>
                            <xsl:attribute name="authority">iso639-2b</xsl:attribute>
                            <xsl:value-of select="//reference/ol"/>
                        </languageTerm>
                    </language>
                </xsl:otherwise>
              </xsl:choose>
            </xsl:if>
            <xsl:if test="//reference/usage/text() [normalize-space(.) ]">
                <accessCondition type="use and reproduction">
                    <xsl:value-of select="//reference/usage"/>
                </accessCondition>
            </xsl:if>
            <xsl:if test="//reference/status/text() [normalize-space(.) ]">
                <physicalDescription>
                    <form authority="local">
                        <xsl:value-of select="//reference/status"/>
                    </form>
                </physicalDescription>
            </xsl:if>
            
            <relatedItem>
                <xsl:attribute name="type">host</xsl:attribute>
                <xsl:if test="//reference/t2/text() [normalize-space(.) ]">
                    <titleInfo>                
                        <xsl:for-each select="//reference/t2">
                            <xsl:call-template name="title2">
                                <xsl:with-param name="strTitle2" select="."/>
                            </xsl:call-template>
                        </xsl:for-each>            
                    </titleInfo>
                </xsl:if>
                <xsl:for-each select="//reference/a2">
                    <name>
                        <xsl:choose>
                            <xsl:when test="contains(text(), ',')">
                                <xsl:attribute name="type">personal</xsl:attribute>
                                <namePart>
                                    <xsl:attribute name="type">given</xsl:attribute>
                                    <xsl:value-of select="normalize-space(substring-after(text(),','))"/>
                                </namePart>
                                <namePart>
                                    <xsl:attribute name="type">family</xsl:attribute>
                                    <xsl:value-of select="normalize-space(substring-before(text(),','))"/>
                                </namePart>
                            </xsl:when>
                            <xsl:otherwise>
                                <xsl:attribute name="type">corporate</xsl:attribute>
                                <namePart>
                                    <!--<xsl:attribute name="type">family</xsl:attribute>--> 
                                    <xsl:value-of select="normalize-space(text())"/>
                                </namePart>
                            </xsl:otherwise>
                        </xsl:choose>
                        <role>
                            <roleTerm>
                                <xsl:attribute name="authority">marcrelator</xsl:attribute>
                                <xsl:attribute name="type">text</xsl:attribute>
                                <xsl:text>Contributor</xsl:text>
                            </roleTerm>
                        </role>
                    </name>
                </xsl:for-each>
                <originInfo>
                    <xsl:if test="//reference/ad/text() [normalize-space(.) ]">
                        <place>
                            <placeTerm>
                                <xsl:attribute name="type">text</xsl:attribute>
                                <xsl:value-of select="//reference/ad"/>
                            </placeTerm>
                        </place>
                    </xsl:if>
                    <xsl:if test="//reference/pp/text() [normalize-space(.) ]">
                        <place>
                            <placeTerm>
                                <xsl:attribute name="type">text</xsl:attribute>
                                <xsl:value-of select="//reference/pp"/>
                            </placeTerm>
                        </place>
                    </xsl:if>
                    <xsl:for-each select="//reference/pb">
                        <publisher>
                            <xsl:value-of select="normalize-space(text())"/>
                        </publisher>
                    </xsl:for-each>
                    <dateIssued>
                        <xsl:attribute name="keyDate">yes</xsl:attribute>
                        <xsl:value-of select="//reference/yr"/>
                    </dateIssued>
                </originInfo>
                <part>
                    <date>
                        <xsl:value-of select="//reference/yr"/>
                    </date>
                    <extent>
                        <xsl:attribute name="unit">page</xsl:attribute>
                        <start>
                            <xsl:value-of select="//reference/sp"/>
                        </start>
                        <end>
                            <xsl:value-of select="//reference/op"/>
                        </end>
                    </extent>
                </part>
                
                <xsl:if test="//reference/sn/text() [normalize-space(.) ]">
                    <xsl:for-each select="//reference/sn">
                        <xsl:call-template name="isbn">
                            <xsl:with-param name="strisbn" select="."/>
                        </xsl:call-template>
                    </xsl:for-each>
                </xsl:if>
            </relatedItem>
            <xsl:if test="//reference/t3/text() [normalize-space(.) ]">
            <relatedItem type="series">
                <originInfo>
                    <issuance>continuing</issuance>
                </originInfo>
                <titleInfo>
                    <title>
                        <xsl:value-of select="//reference/t3"/>
                    </title>
                </titleInfo>
            </relatedItem>
            </xsl:if>    
        </mods>
    </xsl:template>
    
    <xsl:template name="title">
        <xsl:param name="strTitle"/>
        <xsl:choose>
            <xsl:when test="contains($strTitle,':')">
                <title>
                    <xsl:value-of select="normalize-space(substring-before($strTitle,':'))"/>
                </title>
                <subTitle>
                    <xsl:value-of select="normalize-space(substring-after($strTitle,':'))"/>
                </subTitle>
            </xsl:when>
            <xsl:otherwise>
                <title>
                    <xsl:value-of select="normalize-space($strTitle)"/>
                </title>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    
    <xsl:template name="title2">
        <xsl:param name="strTitle2"/>
        <xsl:choose>
            <xsl:when test="contains($strTitle2,':')">
                <title>
                    <xsl:value-of select="normalize-space(substring-before($strTitle2,':'))"/>
                </title>
                <subTitle>
                    <xsl:value-of select="normalize-space(substring-after($strTitle2,':'))"/>
                </subTitle>
            </xsl:when>
            <xsl:otherwise>
                <title>
                    <xsl:value-of select="normalize-space($strTitle2)"/>
                </title>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="links">
        <xsl:param name="str"/>
        <xsl:choose>
            <xsl:when test="contains($str,';')">
                <identifier type="u1">
                    <xsl:value-of select="normalize-space(substring-before($str,';'))"/>
                </identifier>
                <xsl:call-template name="links">
                    <xsl:with-param name="str" select="normalize-space(substring-after($str,';'))"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <identifier type="u1">
                    <xsl:value-of select="normalize-space($str)"/>
                </identifier>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="links2">
        <xsl:param name="str"/>
        <xsl:choose>
            <xsl:when test="contains($str,';')">
                <identifier type="u2">
                    <xsl:value-of select="normalize-space(substring-before($str,';'))"/>
                </identifier>
                <xsl:call-template name="links">
                    <xsl:with-param name="str" select="normalize-space(substring-after($str,';'))"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <identifier type="u2">
                    <xsl:value-of select="normalize-space($str)"/>
                </identifier>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    
    <xsl:template name="isbn">
        <xsl:param name="strisbn"/>
        <xsl:choose>
            <xsl:when test="contains($strisbn,';')">
                <identifier type="isbn">
                    <xsl:value-of select="normalize-space(substring-before($strisbn,';'))"/>
                </identifier>
                <xsl:call-template name="isbn">
                    <xsl:with-param name="strisbn" select="normalize-space(substring-after($strisbn,';'))"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <identifier type="isbn">
                    <xsl:value-of select="normalize-space($strisbn)"/>
                </identifier>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="string-replace-all">
        <xsl:param name="text"/>
        <xsl:param name="replace"/>
        <xsl:param name="by"/>
        <xsl:choose>
            <xsl:when test="contains($text, $replace)">
                <xsl:value-of select="substring-before($text,$replace)"/>
                <xsl:value-of select="$by"/>
                <xsl:call-template name="string-replace-all">
                    <xsl:with-param name="text" select="substring-after($text,$replace)"/>
                    <xsl:with-param name="replace" select="$replace"/>
                    <xsl:with-param name="by" select="$by"/>
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <xsl:value-of select="$text"/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

</xsl:stylesheet>
