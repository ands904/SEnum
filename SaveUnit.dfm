object SaveForm: TSaveForm
  Left = 226
  Top = 211
  BorderStyle = bsToolWindow
  Caption = 'SaveForm'
  ClientHeight = 151
  ClientWidth = 474
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object FilnPanel: TPanel
    Left = 0
    Top = 0
    Width = 475
    Height = 41
    BevelOuter = bvNone
    TabOrder = 0
    DesignSize = (
      475
      41)
    object Label1: TLabel
      Left = 8
      Top = 14
      Width = 63
      Height = 13
      Caption = #1048#1084#1103' '#1092#1072#1081#1083#1072': '
    end
    object FilnEdit: TEdit
      Left = 74
      Top = 10
      Width = 361
      Height = 21
      Anchors = [akLeft, akTop, akRight]
      ReadOnly = True
      TabOrder = 0
    end
    object SelectFilnButton: TButton
      Left = 442
      Top = 7
      Width = 27
      Height = 25
      Anchors = [akTop, akRight]
      Caption = '...'
      TabOrder = 1
      OnClick = SelectFilnButtonClick
    end
  end
  object F1Panel: TPanel
    Left = 0
    Top = 40
    Width = 475
    Height = 37
    BevelOuter = bvNone
    TabOrder = 1
    object Label2: TLabel
      Left = 16
      Top = 9
      Width = 302
      Height = 13
      Caption = #1053#1091#1084#1077#1088#1072#1094#1080#1102' '#1089#1090#1088#1072#1085#1080#1094' '#1087#1088#1086#1080#1079#1074#1086#1076#1080#1090#1100' '#1085#1072#1095#1080#1085#1072#1103' '#1086#1090' '#1092#1072#1081#1083#1072' '#1085#1086#1084#1077#1088':'
    end
    object f1Edit: TEdit
      Left = 324
      Top = 6
      Width = 53
      Height = 21
      TabOrder = 0
      Text = '1'
    end
  end
  object P0Panel: TPanel
    Left = -1
    Top = 77
    Width = 475
    Height = 37
    BevelOuter = bvNone
    TabOrder = 2
    object Label3: TLabel
      Left = 125
      Top = 6
      Width = 193
      Height = 13
      Caption = #1053#1086#1084#1077#1088' '#1087#1077#1088#1074#1086#1081' '#1085#1091#1084#1077#1088#1091#1077#1084#1086#1081' '#1089#1090#1088#1072#1085#1080#1094#1099':'
    end
    object p0Edit: TEdit
      Left = 324
      Top = 4
      Width = 53
      Height = 21
      TabOrder = 0
      Text = '1'
    end
  end
  object ButtonsPanel: TPanel
    Left = -1
    Top = 114
    Width = 475
    Height = 36
    BevelOuter = bvNone
    TabOrder = 3
    object Button1: TButton
      Left = 154
      Top = 5
      Width = 75
      Height = 25
      Caption = 'Ok'
      ModalResult = 1
      TabOrder = 0
    end
    object Button2: TButton
      Left = 258
      Top = 5
      Width = 75
      Height = 25
      Caption = #1054#1090#1084#1077#1085#1072
      ModalResult = 2
      TabOrder = 1
    end
    object BitBtn1: TBitBtn
      Left = 392
      Top = 5
      Width = 75
      Height = 25
      Caption = 'BitBtn1'
      TabOrder = 2
      Visible = False
      OnClick = BitBtn1Click
    end
  end
  object CCreateDialog: TSaveDialog
    DefaultExt = 'doc'
    Filter = #1060#1072#1081#1083#1099' *.doc|*.doc'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofPathMustExist, ofEnableSizing]
    Left = 32
    Top = 104
  end
  object TCreateDialog: TSaveDialog
    DefaultExt = 'tif'
    Filter = #1060#1072#1081#1083#1099' *.tif|*.tif'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofPathMustExist, ofEnableSizing]
    Left = 88
    Top = 112
  end
end
