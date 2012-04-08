<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns="http://www.loc.gov/mods/v3"
    version="1.0">
    
    <xsl:template match="/">
        <mods>
            <titleInfo>
                <title>
                    <xsl:value-of select="//reference/t1"/>
                </title>
                <subTitle>
                    <xsl:value-of select="//reference/t2"/>
                </subTitle>
            </titleInfo>
            <xsl:for-each select="//reference/a1">
                <name>
                    <xsl:attribute name="type">personal</xsl:attribute>
                    <xsl:choose>
                        <xsl:when test="contains(text(), ',')">
                            <namePart>
                                <xsl:attribute name="type">given</xsl:attribute>
                                <xsl:value-of select="normalize-space(substring-after(text(), ','))"/>
                            </namePart>
                            <namePart>
                                <xsl:attribute name="type">family</xsl:attribute>
                                <xsl:value-of select="normalize-space(substring-before(text(), ','))"/>
                            </namePart>
                        </xsl:when>
                        <xsl:otherwise>
                            <namePart>
                                <xsl:attribute name="type">family</xsl:attribute>
                                <xsl:value-of select="normalize-space(text())"/>
                            </namePart>
                        </xsl:otherwise>
                    </xsl:choose>
                        <role>
                            <roleTerm>
                                <xsl:attribute name="authority">marcrelator</xsl:attribute>
                                <xsl:attribute name="type">code</xsl:attribute>author</roleTerm>
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
            <relatedItem>
                <xsl:attribute name="type">host</xsl:attribute>
                <titleInfo>
                    <title>
                        <xsl:value-of select="//reference/jf"/>
                    </title>
                    <title>
                        <xsl:value-of select="//reference/jo"/>
                    </title>
                </titleInfo>
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
                <genre>
                    <xsl:value-of select="//reference/rt"/>
                </genre>
                <xsl:if test="//reference/sn/text() [normalize-space(.) ]">
                    <identifier>
                        <xsl:if test="string-length(//reference/sn) &gt; 10">
                            <xsl:attribute name="type">isbn</xsl:attribute>
                            <xsl:value-of select="//reference/sn"/>
                        </xsl:if>
                        <xsl:if test="string-length(//reference/sn) &lt; 10">
                            <xsl:attribute name="type">issn</xsl:attribute>
                            <xsl:value-of select="//reference/sn"/>
                        </xsl:if>
                    </identifier>
                </xsl:if>
                <identifier>
                    <xsl:attribute name="type">refworks</xsl:attribute>
                    <xsl:value-of select="//reference/id"/>
                </identifier>
            </relatedItem>
            <subject authority="local">
                <xsl:for-each select="//reference/k1">
                    <topic>
                        <xsl:value-of select="normalize-space(text())"/>
                    </topic>
                </xsl:for-each>            
            </subject>
            <abstract>
                <xsl:value-of select="//reference/ab"/>
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
            <part>
                <date>
                    <xsl:value-of select="//reference/yr"/>
                </date>
                <detail>
                    <xsl:attribute name="type">volume</xsl:attribute>
                    <number>
                        <xsl:value-of select="//reference/vo"/>
                    </number>
                </detail>
                <detail>
                    <xsl:attribute name="type">issue</xsl:attribute>
                    <number>
                        <xsl:value-of select="//reference/is"/>
                    </number>
                </detail>
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
            <xsl:if test="//reference/ol/text() [normalize-space(.) ]">
                <language>
                    <languageTerm>
                        <xsl:attribute name="type">code</xsl:attribute>
                        <xsl:attribute name="authority">iso639-2b</xsl:attribute>
                        <xsl:value-of select="//reference/ol"/>
                    </languageTerm>
                </language>
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
        </mods>
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
                    <xsl:value-of select="$str"/>
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
                    <xsl:value-of select="$str"/>
                </identifier>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>
    
</xsl:stylesheet>