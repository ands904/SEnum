object SEnumMainFormM: TSEnumMainFormM
  Left = 13
  Top = 12
  Width = 641
  Height = 373
  Caption = 'SEnumMainFormM'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  Menu = MainMenu1
  OldCreateOrder = False
  OnDestroy = FormDestroy
  OnPaint = FormPaint
  PixelsPerInch = 96
  TextHeight = 13
  object Fog1: TFog
    Left = 0
    Top = 0
    Width = 633
    Height = 33
    Align = alTop
    TabOrder = 0
  end
  object TabControl1: TTabControl
    Left = 24
    Top = 96
    Width = 217
    Height = 73
    TabOrder = 1
  end
  object MainMenu1: TMainMenu
    Top = 152
    object N1: TMenuItem
      Caption = #1060#1072#1081#1083
      object N4: TMenuItem
        Caption = #1044#1086#1073#1072#1074#1080#1090#1100' '#1092#1072#1081#1083#1099' '#1074' '#1089#1087#1080#1089#1086#1082' ...'
      end
      object N7: TMenuItem
        Caption = #1044#1086#1073#1072#1074#1080#1090#1100' '#1074#1089#1077' '#1092#1072#1081#1083#1099' '#1087#1072#1087#1082#1080' '#1074' '#1089#1087#1080#1089#1086#1082' ...'
      end
      object N8: TMenuItem
        Caption = '-'
      end
      object N5: TMenuItem
        Caption = #1055#1088#1086#1089#1090#1072#1074#1080#1090#1100' '#1087#1086#1076#1087#1080#1089#1080' '#1074' '#1092#1072#1081#1083#1072#1093' '#1089#1084#1077#1090#1099' ...'
      end
      object N6: TMenuItem
        Caption = #1057#1086#1079#1076#1072#1090#1100' '#1089#1086#1076#1077#1088#1078#1072#1085#1080#1077' '#1076#1086#1082#1091#1084#1077#1085#1090#1072'  ('#1089#1090#1072#1088#1099#1081' '#1089#1090#1080#1083#1100') ...'
      end
      object N9: TMenuItem
        Caption = #1057#1086#1079#1076#1072#1090#1100' '#1089#1086#1076#1077#1088#1078#1072#1085#1080#1077' '#1076#1086#1082#1091#1084#1077#1085#1090#1072'  ('#1085#1086#1074#1099#1081' '#1089#1090#1080#1083#1100') ...'
      end
      object PDF1: TMenuItem
        Caption = #1055#1077#1095#1072#1090#1072#1090#1100' '#1089#1086#1073#1088#1072#1085#1085#1099#1081' '#1082#1086#1084#1087#1083#1077#1082#1090' '#1074' *.tiff  ...'
      end
      object N10: TMenuItem
        Caption = '-'
      end
      object tiff1: TMenuItem
        Caption = #1056#1077#1076#1072#1082#1090#1080#1088#1086#1074#1072#1090#1100' '#1092#1072#1081#1083' *.tiff'
      end
      object PDFCreator1: TMenuItem
        Caption = 'PDFCreator'
        Visible = False
      end
      object N11: TMenuItem
        Caption = '-'
      end
      object N12: TMenuItem
        Caption = #1042#1099#1093#1086#1076
      end
    end
    object N2: TMenuItem
      Caption = #1055#1086#1084#1086#1097#1100
      object N3: TMenuItem
        Caption = #1054' '#1087#1088#1086#1075#1088#1072#1084#1084#1077
      end
    end
  end
  object Timer1: TTimer
    Left = 344
    Top = 88
  end
end
