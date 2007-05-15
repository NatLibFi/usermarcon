/*
 *  USEMARCON Software - Command Line version
 *  Copyright The British Library, The USEMarcon Consortium, 1995-2000
 *  Adapted by Crossnet Systems Limited - British Library Contract No. BSDS 851
 *
 *  Adapted by ATP Library Systems Ltd, Finland, 2002-2004
 *
 *  File:  tcdlib.cpp
 *
 *  implements a set of functions to manage control data fields
 *
 *  NOTE:  IN NO WAY WHATSOEVER SHOULD THIS FILE BE USED IN THE EARLIER
 *         VERSIONS OF USEMARCON SOFTWARE.
 *
 */

#include <stdlib.h>
#include "tcdlib.h"
#include "error.h"
#include "tools.h"

///////////////////////////////////////////////////////////////////////////////
//
// TCDLib
//
///////////////////////////////////////////////////////////////////////////////
TCDLib::TCDLib(TError *ErrorHandler):TCD(ErrorHandler)
{
}

///////////////////////////////////////////////////////////////////////////////
//
// TCDLib : copie constructeur
//
///////////////////////////////////////////////////////////////////////////////
TCDLib::TCDLib(TCDLib *aCDLib, TCD*aCD):TCD(aCDLib->itsErrorHandler)
{
    _IN=aCDLib->GetIN();
    strcpy(itsTag,aCDLib->GetTag());
    strcpy(itsSubfield,aCDLib->GetSubfield());
    itsTagContainsWildcard = aCDLib->TagContainsWildcard();
    itsTagIsWildcard = aCDLib->TagIsWildcard();
    itsSubfieldContainsWildcard = aCDLib->SubfieldContainsWildcard();

    itsOccurrenceNumber=aCDLib->GetOccurrenceNumber();
    itsTagOccurrenceNumber=aCDLib->GetTagOccurrenceNumber();
    itsSubOccurrenceNumber=aCDLib->GetSubOccurrenceNumber();
    SetBeginning(aCDLib->GetBeginning());
    SetEnd(aCDLib->GetEnd());
    if (aCDLib->GetNext())
    {
        itsNext = (TCD *)new TCDLib((TCDLib*)aCDLib->GetNext());
        itsNext->SetPrevious((TCD*)this);
    }
    else
        itsNext = NULL;
    itsPrevious = NULL;
    SetContent(aCDLib->GetContent(),aCD);
    itsErrorHandler=aCDLib->itsErrorHandler;
}

///////////////////////////////////////////////////////////////////////////////
//
// TCDLib : copie constructeur
//
///////////////////////////////////////////////////////////////////////////////
TCDLib::TCDLib(TCD *aCD):TCD(aCD->GetErrorHandler())
{
    _IN=aCD->GetIN();
    strcpy(itsTag,aCD->GetTag());
    strcpy(itsSubfield,aCD->GetSubfield());
    itsTagContainsWildcard = aCD->TagContainsWildcard();
    itsTagIsWildcard = aCD->TagIsWildcard();
    itsSubfieldContainsWildcard = aCD->SubfieldContainsWildcard();

    itsOccurrenceNumber=aCD->GetOccurrenceNumber();
    itsTagOccurrenceNumber=aCD->GetTagOccurrenceNumber();
    itsSubOccurrenceNumber=aCD->GetSubOccurrenceNumber();
    SetBeginning(aCD->GetBeginning());
    SetEnd(aCD->GetEnd());
    if (aCD->GetNext())
    {
        itsNext = (TCD *)new TCDLib(aCD->GetNext());
        itsNext->SetPrevious((TCD*)this);
    }
    else
        itsNext = NULL;
    itsPrevious = NULL;
    itsErrorHandler=aCD->GetErrorHandler();
}

///////////////////////////////////////////////////////////////////////////////
//
// ~TCDLib
//
///////////////////////////////////////////////////////////////////////////////
TCDLib::~TCDLib(void)
{
}

///////////////////////////////////////////////////////////////////////////////
//
// GetContent
//
// Cette methode retourne le contenu du CDLib correspondant a la description faite
// dans aCD. Trois cas sont possibles :
// - ou bien aCD est NULL, on renvoi simplement le contenu complet du CDLib.
// - ou bien aCD decrit completement le CDLib (c'est-a-dire que le sous-champ est precise),
//   on fournit alors le contenu du CDLib correspondant a l'intervalle specifie dans aCD.
// - ou bien aCD decrit un champ (pas de subfield de defini), auquel cas on doit rendre
//   le contenu complet du champ, en concatenant les contenus de l'ensemble des CDLib
//   correspondant ... et en ne prenant en final que l'intervalle voulu.
//
///////////////////////////////////////////////////////////////////////////////
const char *TCDLib::GetContent(TCD* theCD)
{
    // Si aCD est NULL on retourne simplement le contenu complet du CDLib

    if (theCD==NULL)
        return itsContent.str();

    // Si le sous-champ n'est pas precise, il faut concatener tous les sous-champs
    // suivant, correspondant au champ identifie par aCD.
    // S'ilest precise, il suffit de prendre son contenu

    if (*(theCD->GetSubfield())==0)
    {
        TCDLib* Courant=this;
        *itsErrorHandler->Temporary=0;
        while( Courant && Courant->IsEqual(theCD, GetTagOccurrenceNumber()) )
        {

            // Si le CDLib trouve n'identifie pas un sous-champ, on concatene, sans
            // mettre de balise de sous-champ

            if (*(Courant->GetSubfield())==0)
            {
                if (Courant->GetContent())
                    strcat((char*)itsErrorHandler->Temporary,Courant->GetContent());
            }
            else
            {

                // Si il y a un sous-champ d'identifie, il faut veiller a ne pas prendre
                // les indicateurs. Dans ce cas, on remet la balise marc et son contenu

                if (*(Courant->GetSubfield())!='I')
                {
                    sprintf((char*)itsErrorHandler->Temporary2,"\x1F%c%s",*(Courant->GetSubfield()+1),Courant->GetContent());
                    strcat((char*)itsErrorHandler->Temporary,(char*)itsErrorHandler->Temporary2);
                }
            }
            Courant=(TCDLib*)Courant->GetNext();
        }
    }
    else strcpy((char *)itsErrorHandler->Temporary,itsContent.str());

    // itsErrorHandler->Temporary contient maintenant le contenu du CDLib.
    // On regarde maintenant l'intervalle de position a fournir

    int    aBeginning=theCD->GetBeginning();
    int    aEnd=theCD->GetEnd();

    // Si les positions de sont pas definies, on retourne tout le contenu

    if (aBeginning==0 && aEnd==0) return (char*)itsErrorHandler->Temporary;

    --aBeginning;

    if (itsErrorHandler->GetUTF8Mode())
    {
        if (aBeginning >= 0) 
            aBeginning = utf8_charindex((const char *) itsErrorHandler->Temporary, aBeginning);
        if (aEnd >= 0)
            aEnd = utf8_charindex((const char *) itsErrorHandler->Temporary, aEnd);
    }
    int MaxPos = strlen((char *)itsErrorHandler->Temporary);

    // Si les positions sont definies, on extrait les donnees correspondantes

    if (aBeginning>MaxPos || aEnd>MaxPos || (aEnd>0 && aBeginning>aEnd))
        return "";

    if (aEnd==-1)
        strcpy((char *)itsErrorHandler->Temporary2,(char *)&itsErrorHandler->Temporary[aBeginning]);
    else
    {
        --aEnd;
        memcpy((char *)itsErrorHandler->Temporary2,(char *)&itsErrorHandler->Temporary[aBeginning],aEnd-aBeginning+1);
        itsErrorHandler->Temporary2[aEnd-aBeginning+1]=0;
    }
    return (char *)itsErrorHandler->Temporary2;
}

///////////////////////////////////////////////////////////////////////////////
//
// ResetContent
//
///////////////////////////////////////////////////////////////////////////////
void TCDLib::ResetContent(void)
{
    itsContent.freestr();
}


///////////////////////////////////////////////////////////////////////////////
//
// SetContent
//
// Cette methode remplit le contenu d'un CDLib. Deux cas sont possibles :
// - aCD est NULL, il faut simplement remplir le contenu du CDLib.
// - aCD defini in interval itsBegining -> itsEnd, le contenu passe doit servir
//   a mettre a jour ces positions uniquement.
//
// Le CDLib doit etre unitaire
//
///////////////////////////////////////////////////////////////////////////////
int TCDLib::SetContent(const char *pContent, TCD* aCD)
{
    // Si aCD est NULL on remplit simplement le CDLib (test de validite a effectuer
    // en dehors de la fonction afin d'avoir le contexte de l'erreur).

    if (aCD==NULL)
    {
        if (*itsSubfield=='I' && pContent && pContent[0] != '\0' && pContent[1] != '\0')
            itsErrorHandler->SetErrorD(5005,ERROR,pContent);
        itsContent.str(pContent);
        return 0;
    }


    // Si aCD n'est pas NULL, il faut verifier l'intervalle qu'il definit

    int    aBeginning=aCD->GetBeginning();
    int    aEnd=aCD->GetEnd();

    // Si les positions de debut et fin sont indefinies, on remplit simplement
    // le CDLib en re-appelant cette meme methode sans aCD (NULL par defaut)

    if (aBeginning!=0 || aEnd!=0)

        // Sinon deux cas sont possibles :
        // - ou bien l'intervalle specifie "touche" ou chevauche le contenu actuel
        //   du champ, il suffit donc de completer le champ.
        // - ou bien cet intervalle ne "touche" pas le contenu actuel et on remplit
        //   l'intervalle entre les deux par des blancs

    {
        // aBeginning et aEnd commencent a 1 quand ils sont definis
        --aBeginning;

        if (itsContent.str())
            strcpy((char*)itsErrorHandler->Temporary,itsContent.str());
        else
            *itsErrorHandler->Temporary = '\0';

        if (itsErrorHandler->GetUTF8Mode() && pContent)
        {
            int oldBegining = aBeginning;
            aBeginning = utf8_charindex((char*)itsErrorHandler->Temporary, aBeginning);
            aEnd = utf8_charindex(pContent, aEnd) + aBeginning - oldBegining;
        }
        int MaxPos = strlen((char*)itsErrorHandler->Temporary);

        for (int i = MaxPos; i < aBeginning; itsErrorHandler->Temporary[i++] = ' ');

        if (pContent && *pContent)
        {
            unsigned long pContentLen = strlen(pContent);
            if (aEnd==-1)
            {
                memcpy( (char*)&itsErrorHandler->Temporary[aBeginning], pContent, pContentLen);
                aEnd=aBeginning+pContentLen-1;
            }
            else
            {
                --aEnd;
                // Avoid copying more than we have
                int contentsize = pContentLen + 1;
                int copysize = aEnd - aBeginning + 1;
                if (contentsize < copysize)
                    copysize = contentsize;
                memcpy( (char*)&itsErrorHandler->Temporary[aBeginning], pContent, copysize);
            }
        }
        if (MaxPos<=aEnd) itsErrorHandler->Temporary[aEnd+1]=0;
        SetContent((char*)itsErrorHandler->Temporary);
    }
    else
    {
        SetContent(pContent);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// IsEqual
//
///////////////////////////////////////////////////////////////////////////////
int TCDLib::IsEqual( TCD* aCD, int aTagOccurrenceNumberOverride /*= -1*/ )
{
    // CDLib equals CD provided:

    // 1 - The tags are identical or aCD has a matching wildcard
    char *tag = aCD->GetTag();
    if (tag[0] != '?' && tag[0] != itsTag[0])
        return 0;
    if (tag[0] != '\0')
    {
        if (tag[1] != '?' && tag[1] != itsTag[1])
            return 0;
        if (tag[1] != '\0')
        {
            if (tag[2] != '?' && tag[2] != itsTag[2])
                return 0;
            if (tag[2] != '\0')
            {
                if (tag[3] != '?' && tag[3] != itsTag[3])
                    return 0;
            }
        }
    }

    // 2 - Subfield isn't specified or is identical or aCD has 
    // a matching wildcard
    char *subfield = aCD->GetSubfield();
    if (*subfield)
    {
        if ((subfield[0] != '?' && subfield[0] != itsSubfield[0]) ||
            (subfield[1] != '?' && subfield[1] != itsSubfield[1]))
            return 0;

        // 3 - Subfield occurrence position isn't specified or is identical
        if (aCD->GetSubOccurrenceNumber()>0 && aCD->GetSubOccurrenceNumber()!=itsSubOccurrenceNumber)
            return 0;
    }

    // 4 - Field occurrence position isn't specified or is identical
    if (aCD->GetOccurrenceNumber()>0 && aCD->GetOccurrenceNumber()!=itsOccurrenceNumber)
        return 0;

    // 5 - Tag occurrence position isn't specified or is identical
    int TagOccurrenceNumber = aTagOccurrenceNumberOverride >= 0 ? aTagOccurrenceNumberOverride : aCD->GetTagOccurrenceNumber();
    if (TagOccurrenceNumber>0 && TagOccurrenceNumber!=itsTagOccurrenceNumber)
        return 0;

    return 1;
}

int TCDLib::IsLess( TCD* aCD )
{
    int rc;
    rc=strcmp(itsTag,aCD->GetTag());
    if (rc<0) return 1;
    if (rc>0) return 0;

    if (itsTagOccurrenceNumber<aCD->GetTagOccurrenceNumber()) return 1;
    return 0;
}


///////////////////////////////////////////////////////////////////////////////
//
// NextSubCDLib
//
// Cette methode decoupe le CDLib en sous-CDLibs unitaires, sur les balises de
// sous-champ
//
// Cette methode renvoie 1 si un sous-CDLib a ete trouve, 0 sinon
//
///////////////////////////////////////////////////////////////////////////////
int TCDLib::NextSubCDLib(TCDLib** pCDLib, int* _pos, char* defst)
{
    TCDLib* aCDLib;
    int     first;
    int     pos=*_pos, startpos;
    char    tmp[3];

    if (!itsContent.str())
    {
        return 0;
    }

    tmp[0]='$';
    tmp[2]=0;
    first=(pos==0);

    // On parcourt tous les caracteres du contenu du CDLib, a partir de la position
    // passee en parametre

    while(itsContent.str()[pos])
    {
        if (itsContent.str()[pos]==0x1F && itsContent.str()[pos+1]!=0)
        {
            if (first && pos && *defst)
            {
                // Si on est a la premiere balise et qu'on n'est pas au debut du champ
                // et qu'un ss-champ par defaut est specifie, on le rempli avec le debut du champ
                tmp[1]=defst[1];
                startpos = 0;
                memcpy((char*)itsErrorHandler->Temporary,itsContent.str(),pos);
                itsErrorHandler->Temporary[pos]=0;
            }
            else
            {
                // Si on trouve une balise, on recopie le contenu du sous-champ dans
                // itsErrorHandler->Temporary, jusqu'a la balise suivante

                tmp[1]=itsContent.str()[pos+1];
                pos+=2;
                startpos=pos;
                while(itsContent.str()[pos] && itsContent.str()[pos]!=0x1F) ++pos;
                memcpy((char*)itsErrorHandler->Temporary,&itsContent.str()[startpos],pos-startpos);
                itsErrorHandler->Temporary[pos-startpos]=0;
            }


            // On cree un nouveau CDLib correspondant au sous-champ trouve
            first=0;
            aCDLib=new TCDLib(itsErrorHandler);
            aCDLib->SetTag(itsTag);
            aCDLib->SetTagOccurrenceNumber(itsTagOccurrenceNumber);
            aCDLib->SetSubfield(tmp);
            aCDLib->SetContent((char*)itsErrorHandler->Temporary);
            if (!aCDLib->GetContent())
                itsErrorHandler->SetErrorD(5000,ERROR,"While setting content of a new sub-cdlib");

            // Find out the subfield occurrence number (ns)
            int ns = 0;
            for (int p2 = startpos; p2 > 0; p2--)
            {
                if (itsContent.str()[p2] == tmp[1] && itsContent.str()[p2 - 1] == 0x1F)
                    ++ns;
            }
            aCDLib->SetSubOccurrenceNumber(ns);

            *pCDLib=aCDLib;
            *_pos=pos;
            return 1;
        }
        ++pos;
    }

    *_pos=pos;
    return 0;
}

