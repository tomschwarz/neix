/**
 * Feed parser class.
 *
 * @package     CRSS
 * @author      Thomas Schwarz
 * @copyright   Copyright (c) 2020, Thomas Schwarz
 * @license     -
 * @since       Version 0.1.0
 * @filesource
 */

#include <iostream>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <regex>

#include "rapidxml/rapidxml.hpp"
#include "parser/Parser.h"

using namespace rapidxml;
using namespace crss;

/**
 * Constructor
 */
Parser::Parser(struct rawRss content)
{
   this->setRawRss(content);
}


/**
 * Destructor
 */
Parser::~Parser() = default;


/**
 * Set raw rss content
 *
 * @param rawContent    - The loaded raw content
 */
void Parser::setRawRss(struct rawRss rawContent)
{
    this->rss = &rawContent;

    std::string s(this->rss->content);
    s.erase(std::remove(s.begin(), s.end(), '\n'), s.end());
    char *temp = const_cast<char *>(s.c_str());

    this->xmlDocument.parse<0>(temp);
    this->rootNode = this->xmlDocument.first_node();
    this->entryNode = nullptr;
}


char * Parser::getNodeContent(std::string nodeName)
{
    char *nodeValue = nullptr;
    char *content = (char*) calloc(1, sizeof(char));
    xml_node<> *xmlNode = nullptr;
    xmlNode = this->entryNode->first_node(nodeName.c_str());

    if (xmlNode)
    {
        nodeValue = xmlNode->value();
        content = (char*) calloc(strlen(nodeValue), sizeof(char));
        strcpy(content, nodeValue);
    }
    else
    {
        strcpy(content, "");
    }

    return content;
}


/**
 * Get an feed item of raw content
 *
 * @return  Item of feed
 */
struct rssItem* Parser::getFeedItem()
{
    struct rssItem *item = (struct rssItem*) calloc(1, sizeof(struct rssItem));
    item->read = 0;
    if (this->entryNode == nullptr)
    {
        this->entryNode = this->rootNode->first_node("entry");
    }
    else
    {
        this->entryNode = this->entryNode->next_sibling();
    }

    // Get feed title
    item->title = this->getNodeContent("title");
    item->title = this->convertHtmlToPlaintext(item->title);

    // Get feed content
    item->description = this->getNodeContent("summary");
    if (strlen(item->description) == 0)
    {
        item->description = this->getNodeContent("content");
    }
    item->description = this->convertHtmlToPlaintext(item->description);

    // Get feed link
    char *url = this->entryNode->first_node("link")->first_attribute("href")->value();
    item->url = strdup(url);

    // Get feed date
    item->date = this->getNodeContent("updated");
    item->date = this->formatTimeString(item->date);

    return item;
}


/**
 * Convert given text to plaintext
 * > removes html tags
 *
 * @param   text    - The text which should be parsed
 * @return
 */
char * Parser::convertHtmlToPlaintext(char *text)
{
    char *plaintext;
    std::regex regex("<[^>]*>");
    std::string convertedText;

    convertedText = std::regex_replace(text, regex, "");
    plaintext = strdup(convertedText.c_str());

    return plaintext;
}


/**
 * Format given times string to configured format
 *
 * @param   timeString
 * @return  Formatted time string
 */
char* Parser::formatTimeString(const char *timeString)
{
    std::stringstream date(timeString);
    std::ostringstream formattedTimeString;
    struct std::tm when{0};
    memset(&when, 0, sizeof(when));

    date >> std::get_time(&when,"%Y-%m-%dT%H:%M:%S");
    formattedTimeString << std::put_time(&when, "%d.%m.%Y %H:%M"); // TODO: get format from config

    std::string tmp(formattedTimeString.str());
    char *formattedDate = strdup(tmp.c_str());

    return formattedDate;
}