<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:mads="http://www.loc.gov/mads/v2" exclude-result-prefixes="mads"
    xmlns:dc="http://purl.org/dc/elements/1.1/"
    xmlns:srw_dc="info:srw/schema/1/dc-schema"
    xmlns:oai_dc="http://www.openarchives.org/OAI/2.0/oai_dc/"
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    
    <xsl:output method="xml" indent="yes"/>
    <xsl:strip-space elements="*"/>
    <xsl:template match="/">
        <xsl:element name="dc:title">
            <xsl:value-of select="//mads:namePart"/>
        </xsl:element>   
    </xsl:template>
    
    <!-- suppress all else:-->
    <xsl:template match="*"/>
    
</xsl:stylesheet>