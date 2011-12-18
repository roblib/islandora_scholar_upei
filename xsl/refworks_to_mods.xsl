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
                    <namePart>
                        <xsl:attribute name="type">given</xsl:attribute>
                        <xsl:value-of select="normalize-space(substring-after(text(), ','))"/>
                    </namePart>
                    <namePart>
                        <xsl:attribute name="type">family</xsl:attribute>
                        <xsl:value-of select="normalize-space(substring-before(text(), ','))"/>
                    </namePart>
                    <role>
                        <roleTerm>
                            <xsl:attribute name="authority">marcrelator</xsl:attribute>
                            <xsl:attribute name="type">code</xsl:attribute>aut</roleTerm>
                    </role>
                </name>
            </xsl:for-each>
            <name>
                <xsl:attribute name="type">corporate</xsl:attribute>
                <xsl:for-each select="//reference/u1">
                    <namePart>
                        <xsl:value-of select="normalize-space(text())"/>
                    </namePart>
                </xsl:for-each>
            </name>
            <subject>
                <xsl:attribute name="authority">local</xsl:attribute>
                <xsl:for-each select="//reference/u2">
                    <topic>
                        <xsl:value-of select="normalize-space(text())"/>
                    </topic>
                </xsl:for-each>
            </subject>
            <originInfo>
                <dateIssued>
                    <xsl:value-of select="//reference/yr"/>
                </dateIssued>
            </originInfo>
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
                <xsl:if test="//reference/ad/text() [normalize-space(.) ]">
                    <originInfo>
                        <place>
                            <placeTerm>
                                <xsl:attribute name="type">text</xsl:attribute>
                                <xsl:value-of select="//reference/ad"/>
                            </placeTerm>
                        </place>
                        <xsl:if test="//reference/pp/text() [normalize-space(.) ]">
                            <place>
                                <placeTerm>
                                    <xsl:attribute name="type">text</xsl:attribute>
                                    <xsl:value-of select="//reference/pp"/>
                                </placeTerm>
                            </place>
                        </xsl:if>
                    </originInfo>
                </xsl:if>
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
            <abstract>
                <xsl:value-of select="//reference/ab"/>
            </abstract>
            <xsl:if test="//reference/no/text() [normalize-space(.) ]">
                <note>
                    <xsl:value-of select="//reference/no"/>
                </note>
            </xsl:if>
            <note>Source type: <xsl:value-of select="//reference/sr"/></note>
            <location>
                <url>
                    <xsl:value-of select="concat(//reference/ul, '; ', //reference/lk)"/>
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
        </mods>

    </xsl:template>

</xsl:stylesheet>
