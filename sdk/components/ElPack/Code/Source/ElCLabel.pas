{====================================================}
{                                                    }
{   EldoS Visual Components                          }
{                                                    }
{   Copyright (c) 1998-2002, EldoS                   }
{                                                    }
{====================================================}

(*

Version History

05/27/2001 (c) Akzhan Abdulin

    This component fixes Color property design-time storing issue
    of its ancestor, TLabel (clWindow not stored)

*)
{$include elpack2.inc}
{$ifdef ELPACK_SINGLECOMP}
{$I ElPack.inc}
{$else}
{$ifdef LINUX}
{$I ../ElPack.inc}
{$else}
{$I ..\ElPack.inc}
{$endif}
{$endif}
unit ElCLabel;

interface

uses
{$ifndef CLX_USED}
  StdCtrls;
{$else}
  QStdCtrls;
{$endif}

type
  TElCustomLabel = class(TLabel)
  public
    procedure Loaded; override;
  published
    property Color nodefault;
  end;

implementation

{ TElCustomLabel }

procedure TElCustomLabel.Loaded;
begin
  inherited;
  if ParentColor then
  begin
    ParentColor := False;
    ParentColor := True;
  end;
end;

end.
