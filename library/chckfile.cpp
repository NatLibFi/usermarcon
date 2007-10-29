/*
 *  USEMARCON Software - Command Line version
 *  Copyright The British Library, The USEMarcon Consortium, 1995-2000
 *  Adapted by Crossnet Systems Limited - British Library Contract No. BSDS 851
 *
 *  Adapted by ATP Library Systems Ltd, Finland, 2002-2004
 *  Adapted by The National Library of Finland, 2004-2007
 *
 *  File:  chckfile.cpp
 *
 *  implements a class to manage check files
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "error.h"
#include "tools.h"
#include "chckfile.h"


///////////////////////////////////////////////////////////////////////////////
//
// TCheckFile
//
///////////////////////////////////////////////////////////////////////////////
TCheckFile::TCheckFile(typestr & FileSpec, TError *ErrorHandler)
 : TFile(FileSpec, ErrorHandler)
{
    itsFirstCheckField  = itsLastCheckField = NULL;
}

///////////////////////////////////////////////////////////////////////////////
//
// ~TCheckFile
//
///////////////////////////////////////////////////////////////////////////////
TCheckFile::~TCheckFile()
{
    DelTreeCheckTag();
}

///////////////////////////////////////////////////////////////////////////////
//
// Close
//
///////////////////////////////////////////////////////////////////////////////
int TCheckFile::Close()
{
    DelTreeCheckTag();
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// Open
//
///////////////////////////////////////////////////////////////////////////////
int TCheckFile::Open(int IO)
{
    typestr         aLine;
    typestr         aLineCopy;
    char            *Pointer = NULL;
    typestr         aSpec;
    int             AnalysedControl=1;
    int             Line;
    TControlField   *OldControl=itsLastCheckField;

    // Ouverture du fichier
    if (TFile::Open())
        return itsErrorHandler->GetErrorCode();

    // Lecture de chaque ligne du fichier
    while (NextLine(&aLine, &aSpec, &Line) == 0)
    {
        // Si la ligne n'est pas vide, la traiter
        if (RemoveSpace(aLine.str()))
        {
            aLineCopy = aLine;
            // Extraction du Tag de la regle de controle
            if ((Pointer=strtok(aLine.str(),"|"))==NULL)
                // Regle de controle invalide ==> passage a la regle suivante
            {
                itsErrorHandler->SetErrorD(IO==INPUT ? 2001 : 7001, WARNING, get_error(aSpec, Line, aLineCopy).str());
                continue;
            }
            if ((RemoveSpace(Pointer)!=4) || ((Pointer[3]!='_') && (Pointer[3]!='+') && (Pointer[3]!='?') && (Pointer[3]!='*')))
                // Le Tag de la Regle de controle est invalide ==> passage a la regle suivante
            {
                itsErrorHandler->SetErrorD(IO==INPUT ? 2002 : 7002, WARNING, get_error(aSpec, Line, aLineCopy).str());
                continue;
            }

            // Memorisation de la nouvelle regle de controle
            if (AnalysedControl)
                // Creation d'un nouveau TControlField si la regle precedente a ete correctement analys权e
            {
                if (!itsLastCheckField)
                    // Aucune regle de controle n'a encore ete chargee
                {
                    OldControl = NULL;
                    itsFirstCheckField = itsLastCheckField = new TControlField(itsErrorHandler);
                    if (!itsLastCheckField)
                    {
                        // Echec de la cr权ation d'un TControlField ==> Arret et fermeture du fichier
                        itsErrorHandler->SetErrorD(IO==INPUT ? 2501 : 7501, ERROR, get_error(aSpec, Line, aLineCopy).str());
                        break;
                    }
                }
                else
                    // une regle de controle existe deja ==> on verifie que cette nouvelle regle n'existe pas d权j柔
                    if (!FindControlField(Pointer))
                        // regle de controle non encore rencontree ==> ajout en fin de liste
                    {
                        itsLastCheckField->SetNextTag(new TControlField(itsErrorHandler));
                        if (!itsLastCheckField->GetNextTag())
                        {
                            // Echec de la cr权ation d'un TControlField ==> Arret et fermeture du fichier
                            itsErrorHandler->SetErrorD(IO==INPUT ? 2501 : 7501, ERROR, get_error(aSpec, Line, aLineCopy).str());
                            break;
                        }
                        OldControl = itsLastCheckField;
                        itsLastCheckField = itsLastCheckField->GetNextTag();
                    }
                    else
                        // regle ignor权e car d权j柔 existante ==> passage a la regle suivante
                    {
                        itsErrorHandler->SetErrorD(IO==INPUT ? 2003 : 7003, WARNING, get_error(aSpec, Line, aLineCopy).str());
                        continue;
                    }
            }
            else
                // La regle precedente n'ayant pas ete enregistree ==> on effectue une RAZ du TControlField courant pour le r权utiliser
                ResetControl(itsLastCheckField);

            // Memorisation du Tag et des caracteristiques du Tag de la regle de controle
            itsLastCheckField->SetTag(Pointer);
            switch (Pointer[3])
            {
            case '_': // Oblig / !Repet
                itsLastCheckField->SetTagMandatory(1);
                itsLastCheckField->SetTagRepeatable(0);
                break;
            case '+': // Oblig / Repet
                itsLastCheckField->SetTagMandatory(1);
                itsLastCheckField->SetTagRepeatable(1);
                break;
            case '?': // !Oblig / !Repet
                itsLastCheckField->SetTagMandatory(0);
                itsLastCheckField->SetTagRepeatable(0);
                break;
            case '*': // !Oblig / Repet
                itsLastCheckField->SetTagMandatory(0);
                itsLastCheckField->SetTagRepeatable(1);
                break;
            }

            // Extraction de la liste des premiers indicateurs possibles
            if ((Pointer=strtok(NULL,"|"))==NULL)
                // Regle de controle invalide ==> passage a la regle suivante
            {
                itsErrorHandler->SetErrorD(IO==INPUT ? 2001 : 7001, WARNING, get_error(aSpec, Line, aLineCopy).str());
                AnalysedControl=0;
                continue;
            }
            if ((!RemoveSpace(Pointer)) || (memcmp(Pointer,"I1=",3)))
                // Le Tag de la Regle de controle est invalide ==> passage a la regle suivante
            {
                itsErrorHandler->SetErrorD(IO==INPUT ? 2004 : 7004, WARNING, get_error(aSpec, Line, aLineCopy).str());
                AnalysedControl=0;
                continue;
            }
            // Memorisation des Indicateurs 1
            if (Pointer[3])
            {
                CodeHexaToChar(&Pointer[3]);
                itsLastCheckField->SetFirstIndicators(&Pointer[3]);
            }

            // Extraction de la liste des deuxiemes indicateurs possibles
            if ((Pointer=strtok(NULL,"|"))==NULL)
                // Regle de controle invalide ==> passage a la regle suivante
            {
                itsErrorHandler->SetErrorD(IO==INPUT ? 2001 : 7001, WARNING, get_error(aSpec, Line, aLineCopy).str());
                AnalysedControl=0;
                continue;
            }
            if ((!RemoveSpace(Pointer)) || (memcmp(Pointer,"I2=",3)))
                // Le Tag de la Regle de controle est invalide ==> passage a la regle suivante
            {
                itsErrorHandler->SetErrorD(IO==INPUT ? 2005 : 7005, WARNING, get_error(aSpec, Line, aLineCopy).str());
                AnalysedControl=0;
                continue;
            }
            // Memorisation des Indicateurs 2
            if (Pointer[3])
            {
                CodeHexaToChar(&Pointer[3]);
                itsLastCheckField->SetSecondIndicators(&Pointer[3]);
            }

            // Extraction des differents sous-champs
            while (((Pointer=strtok(NULL,"|"))!=NULL) && (AnalysedControl))
            {
                if ((RemoveSpace(Pointer)!=3) || ((Pointer[0]!='$') && ((!isalnum(Pointer[1])) && (Pointer[1]!='*')) &&
                    (Pointer[2]!='_') && (Pointer[2]!='+') && (Pointer[2]!='?') && (Pointer[2]!='*')))
                    // Le Sub de la Regle de controle est invalide ==> passage a la regle suivante
                {
                    itsErrorHandler->SetErrorD(IO==INPUT ? 2006 : 7006, WARNING, get_error(aSpec, Line, aLineCopy).str());
                    AnalysedControl=0;
                    continue;
                }
                if (!itsLastCheckField->GetFirstSubfield())
                    // aucun sous-champ n'est encore present
                {
                    itsLastCheckField->SetFirstSubfield(new TCtrlSubfield(itsErrorHandler));
                    if (!itsLastCheckField->GetFirstSubfield())
                        // Echec de la creation d'un nouveau TCtrlSubfield
                    {
                        itsErrorHandler->SetErrorD(IO==INPUT ? 2502 : 7502, ERROR, get_error(aSpec, Line, aLineCopy).str());
                        AnalysedControl=0;
                        continue;
                    }
                    itsLastCheckField->SetLastSubfield(itsLastCheckField->GetFirstSubfield());
                }
                else
                    // il existe d权j柔 une liste de sous-champs
                {
                    itsLastCheckField->GetLastSubfield()->SetNextSub(new TCtrlSubfield(itsErrorHandler));
                    if (!itsLastCheckField->GetLastSubfield()->GetNextSub())
                        // Echec de la creation d'un nouveau TCtrlSubfield
                    {
                        itsErrorHandler->SetErrorD(IO==INPUT ? 2502 : 7502, ERROR, get_error(aSpec, Line, aLineCopy).str());
                        AnalysedControl=0;
                        continue;
                    }
                    itsLastCheckField->SetLastSubfield(itsLastCheckField->GetLastSubfield()->GetNextSub());
                }
                // Memorisation du Tag et des caracteristiques du Tag de la regle de controle
                itsLastCheckField->GetLastSubfield()->SetSub(Pointer[1]);
                switch (Pointer[2])
                {
                case '_': // Oblig / !Repet
                    itsLastCheckField->GetLastSubfield()->SetSubMandatory(1);
                    itsLastCheckField->GetLastSubfield()->SetSubRepeatable(0);
                    break;
                case '+': // Oblig / Repet
                    itsLastCheckField->GetLastSubfield()->SetSubMandatory(1);
                    itsLastCheckField->GetLastSubfield()->SetSubRepeatable(1);
                    break;
                case '?': // !Oblig / !Repet
                    itsLastCheckField->GetLastSubfield()->SetSubMandatory(0);
                    itsLastCheckField->GetLastSubfield()->SetSubRepeatable(0);
                    break;
                case '*': // !Oblig / Repet
                    itsLastCheckField->GetLastSubfield()->SetSubMandatory(0);
                    itsLastCheckField->GetLastSubfield()->SetSubRepeatable(1);
                    break;
                }
            }
     }

     // Regle correctement analys权e
     AnalysedControl = 1;
  }

  // supression du dernier TControlField s'il est incorrect
  if (!AnalysedControl)
  {
      delete itsLastCheckField;
      itsLastCheckField = OldControl;
  }

  // On referme le fichier
  TFile::Close();

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// Verify
//
///////////////////////////////////////////////////////////////////////////////
int TCheckFile::Verify(int IO,TUMRecord *aRecord)
{
    TMarcField      *CurrentField = NULL;
    TControlField   *CurrentControl = NULL;
    TCtrlSubfield   *CurrentCtrlSub = NULL;
    int    Position = 0;
    char   Balise;
    bool   AtLeastOneSub = false;

    CurrentField = aRecord->GetFirstField();
    while (CurrentField)
    {
        // Find control field for current field
        if ((CurrentControl=FindControlField(CurrentField->GetTag()))!=NULL)
        {
            // Check repeatability
            if ((!CurrentControl->GetTagRepeatable()) && (CurrentControl->GetTagOccurrency()))
            {
                // Non-repeatable field repeated
                char rec[50];
                sprintf(rec, "(record %ld) ", itsErrorHandler->GetRecordNumber());
                typestr tmp = typestr(rec) + typestr("Field ") + CurrentField->GetTag() + ": '" + CurrentField->GetLib1() + "'";
                itsErrorHandler->SetErrorD(IO==INPUT ? 2101 : 7101, WARNING, tmp.str());
            }
            else
            {
                // Update control field
                CurrentControl->SetTagOccurrency();
            }

            // Check first indicator
            if (TestIndicator(CurrentField->GetI1(),CurrentControl->GetFirstIndicators()))
            {
                if (CurrentField->GetI1())
                {
                    // Warning only if the indicator value is invalid
                    // (passing empty values as proper ones)
                    char rec[50];
                    sprintf(rec, "(record %ld) ", itsErrorHandler->GetRecordNumber());
                    typestr tmp = typestr(rec) + typestr("Field ") + CurrentField->GetTag() + ": '" + CurrentField->GetLib1() + "'";
                    tmp += " (ind '";
                    tmp += CurrentField->GetI1();
                    tmp += "')";
                    itsErrorHandler->SetErrorD(IO==INPUT ? 2102 : 7102, WARNING, tmp.str());
                }
            }

            // Check second indicator
            if (TestIndicator(CurrentField->GetI2(),CurrentControl->GetSecondIndicators()))
            {
                if (CurrentField->GetI2())
                {
                    // Warning only if the indicator value is invalid
                    // (passing empty values as proper ones)
                    char rec[50];
                    sprintf(rec, "(record %ld) ", itsErrorHandler->GetRecordNumber());
                    typestr tmp = typestr(rec) + typestr("Field ") + CurrentField->GetTag() + ": '" + CurrentField->GetLib1() + "'";
                    tmp += " (ind '";
                    tmp += CurrentField->GetI2();
                    tmp += "')";
                    itsErrorHandler->SetErrorD(IO==INPUT ? 2103 : 7103, WARNING, tmp.str());
                }
            }

            // Check subfields
            Position=0;
            Balise='*';
            while (CurrentField->NextSubField(&Position, &Balise))
            {
                if (TestSubfield(Balise,CurrentControl->GetFirstSubfield()))
                {
                    // Invalid or redundant subfield
                    char rec[50];
                    sprintf(rec, "(record %ld) ", itsErrorHandler->GetRecordNumber());
                    typestr tmp = typestr(rec) + typestr("Field ") + CurrentField->GetTag() + ": '" + CurrentField->GetLib1() + "'";
                    tmp += " (subfield '$";
                    tmp += Balise;
                    tmp += "')";
                    itsErrorHandler->SetErrorD(IO==INPUT ? 2104 : 7104, WARNING, tmp.str());
                }
                else
                    AtLeastOneSub = true;
                Balise='*';
            }
            // Check that there is at least one subfield in each field (ignoring control fields)
            if ((!AtLeastOneSub) && (CurrentControl->GetFirstSubfield()))
            {
                char rec[50];
                sprintf(rec, "(record %ld) ", itsErrorHandler->GetRecordNumber());
                typestr tmp = typestr(rec) + typestr("Field ") + CurrentField->GetTag() + ": '" + CurrentField->GetLib1() + "'";
                itsErrorHandler->SetErrorD(IO==INPUT ? 2108 : 7108, WARNING, tmp.str());
            }
            CurrentCtrlSub=CurrentControl->GetFirstSubfield();
            while(CurrentCtrlSub)
            {
                if ((CurrentCtrlSub->GetSubMandatory()) && (!CurrentCtrlSub->GetSubOccurency()))
                {
                    // A mandatory subfield is missing
                    char rec[50];
                    sprintf(rec, "(record %ld) ", itsErrorHandler->GetRecordNumber());
                    typestr tmp = typestr(rec) + typestr("Field ") + CurrentField->GetTag() + ": '" + CurrentField->GetLib1() + "'";
                    tmp += " (subfield '$";
                    tmp += CurrentCtrlSub->GetSub();
                    tmp += "')";
                    itsErrorHandler->SetErrorD(IO==INPUT ? 2107 : 7107, WARNING, tmp.str());
                }
                CurrentCtrlSub->SetSubOccurency(0);
                CurrentCtrlSub=CurrentCtrlSub->GetNextSub();
            }
            CurrentControl->ResetSubOccurency(CurrentControl->GetFirstSubfield());
        }
        else
        {
            // Unexpected field 
            char rec[50];
            sprintf(rec, "(record %ld) ", itsErrorHandler->GetRecordNumber());
            typestr tmp = typestr(rec) + typestr("Field ") + CurrentField->GetTag() + ": '" + CurrentField->GetLib1() + "'";
            itsErrorHandler->SetErrorD(IO==INPUT ? 2105 : 7105, WARNING, tmp.str());
        }
        CurrentField = CurrentField->GetNextField();
    }

    // Check presense of mandatory fields
    CurrentControl = GetFirstCheckField();
    while (CurrentControl)
    {
        if ((CurrentControl->GetTagMandatory()) && (!CurrentControl->GetTagOccurrency()))
        {
            // Missing mandatory field
            char rec[50];
            sprintf(rec, "(record %ld) ", itsErrorHandler->GetRecordNumber());
            typestr tmp = typestr(rec) + "Field ";
            tmp += CurrentControl->GetTag();
            itsErrorHandler->SetErrorD(IO==INPUT ? 2106 : 7106, WARNING, tmp.str());
        }
        CurrentControl->SetTagOccurrency(0);
        CurrentControl=CurrentControl->GetNextTag();
    }

    return itsErrorHandler->GetErrorCode();
}

///////////////////////////////////////////////////////////////////////////////
//
// TestIndicator
//
///////////////////////////////////////////////////////////////////////////////
int TCheckFile::TestIndicator(char Ind,char *IndList)
{
    unsigned int    Indice;

    if ((Ind) && (!IndList))
        // le champ comporte un indicateur malgr瓤s qu'aucun indicateur ne soit pr权vu ==> erreur
        return 1;

    if ((!Ind) && (IndList))
        // le champ ne comporte pas d'indicateur alors qu'il le devrait ==> erreur
        return 1;

    if ((!Ind) && (!IndList))
        // le champ n'a pas d'indicateur et il n'en est de toutes fa群on pr权vu aucun ==> correct
        return 0;

    for (Indice=0; Indice<strlen(IndList); Indice++)
        if ((Ind==IndList[Indice]) || ((Ind==' ') && (IndList[Indice]=='_')) || (IndList[Indice]=='*'))
            // indicateur correct
            return 0;

        // indicateur incorrect
        return 1;
}

///////////////////////////////////////////////////////////////////////////////
//
// TestSubfield
//
///////////////////////////////////////////////////////////////////////////////
int TCheckFile::TestSubfield(char Balise,TCtrlSubfield *SubList)
{
    TCtrlSubfield   *CurrentSub;

    CurrentSub = SubList;

    if (!CurrentSub)
        // le champ comporte un sous-champ malgr瓤s le fait que ce champ ne devrait pas en avoir
        return 1;

    while (CurrentSub)
    {
        if (CurrentSub->GetSub()==Balise)
            // balise trouv权e parmi la liste des sous-champs possibles
            if ((!CurrentSub->GetSubRepeatable()) && (CurrentSub->GetSubOccurency()))
                // ce sous-champ, non r权p权table, a d权j柔 权t权 rencontr权 ==> Occurence superflue
                return 1;
            else
                // Sous-champ trouv权 parmi les sous-champs autoris权s ==> Comptabilisation de cette occurence de sous-champ
            {
                CurrentSub->SetSubOccurency();
                return 0;
            }
            else
                // balise non trouv权e
                if (CurrentSub->GetSub()=='*')
                    // n'importe quelle balise est autoris权e
                    return 0;

                CurrentSub=CurrentSub->GetNextSub();
    }

    // le sous-champ recherch权 ne fait pas partie de la liste des sous-champs possibles
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
//
// FindControlField
//
///////////////////////////////////////////////////////////////////////////////
TControlField *TCheckFile::FindControlField(char *aTag)
{
    TControlField   *CurrentControl;

    CurrentControl = itsFirstCheckField;
    while ((CurrentControl) && (strcmp(CurrentControl->GetTag(),aTag)))
        CurrentControl = CurrentControl->GetNextTag();
    return CurrentControl;
}

///////////////////////////////////////////////////////////////////////////////
//
// ResetControl
//
///////////////////////////////////////////////////////////////////////////////
void TCheckFile::ResetControl(TControlField *Control)
{
    Control->SetTag("");
    Control->SetTagMandatory(0);
    Control->SetTagRepeatable(0);
    Control->SetFirstIndicators("");
    Control->SetSecondIndicators("");
    Control->DelTreeCheckSub();
    Control->SetNextTag(NULL);
}

///////////////////////////////////////////////////////////////////////////////
//
// DelTreeCheck
//
///////////////////////////////////////////////////////////////////////////////
void TCheckFile::DelTreeCheckTag(void)
{
    TControlField   *Courant,
        *Suivant;

    Courant=itsFirstCheckField;
    while(Courant)
    {
        Suivant=Courant->GetNextTag();
        Courant->DelTreeCheckSub();
        delete Courant;
        Courant=Suivant;
    }
    itsFirstCheckField = itsLastCheckField = NULL;
}

typestr TCheckFile::get_error(typestr & a_filename, int a_lineNumber, typestr & a_line)
{
    typestr errorstr;
    errorstr.allocstr(strlen(a_filename.str()) + 100 + strlen(a_line.str()));
    sprintf(errorstr.str(), "in file '%s' at line %d : \n%s", a_filename.str(), a_lineNumber, 
        a_line.str());

    return errorstr;
}
